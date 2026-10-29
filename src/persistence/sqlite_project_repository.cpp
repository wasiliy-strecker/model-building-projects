#include "persistence/sqlite_project_repository.h"
#include "application/photo_importer.h"
#include <QCoreApplication>

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>
#include <utility>

namespace modelbuild {
namespace {

QString schemaSql() {
  return QStringLiteral(
      "CREATE TABLE projects (id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "name TEXT NOT NULL, manufacturer TEXT NOT NULL DEFAULT '', "
      "location TEXT NOT NULL DEFAULT '', note TEXT NOT NULL DEFAULT '', "
      "status TEXT NOT NULL DEFAULT 'planned' CHECK(status IN ('planned','started','finished')), "
      "photo BLOB CHECK(photo IS NULL OR (typeof(photo)='blob' AND length(photo) BETWEEN 1 AND "
      "10485760)))");
}

ProjectStatus readStatus(const QVariant& value) {
  try {
    return statusFromStorage(value.toString());
  } catch (const ValidationError& error) {
    throw StorageError(QString::fromUtf8(error.what()));
  }
}

[[noreturn]] void fail(const QSqlError& error) {
  // SQLite extended result codes keep the primary result in the low byte.
  const int code = error.nativeErrorCode().toInt() & 0xff;
  if (code == 5 || code == 6) {
    throw StorageError(QCoreApplication::translate(
        "App", "Die Datenbank ist gerade gesperrt. Beende andere Zugriffe "
               "und versuche es erneut. Deine Eingaben bleiben erhalten."));
  }
  if (code == 11 || code == 26) {
    throw StorageError(QCoreApplication::translate(
        "App", "Die Datenbank ist beschädigt oder keine gültige SQLite-Datei. "
               "Sie wird nicht ersetzt. Sichere die Datei vor einer Reparatur."));
  }
  if (code == 3 || code == 8 || code == 14) {
    throw StorageError(QCoreApplication::translate(
        "App", "Die Datenbank kann nicht geöffnet oder beschrieben werden. "
               "Prüfe das Datenverzeichnis und deine Zugriffsrechte."));
  }
  if (code == 13) {
    throw StorageError(QCoreApplication::translate("App",
                                                   "Es ist nicht genügend Speicherplatz verfügbar. "
                                                   "Schaffe Platz und versuche es erneut."));
  }
  throw StorageError(QCoreApplication::translate(
                         "App", "Der Datenbankzugriff ist fehlgeschlagen. "
                                "Prüfe Datenträger und Zugriffsrechte und versuche es erneut.\n%1")
                         .arg(error.text()));
}

void execute(QSqlQuery& query, const QString& sql) {
  if (!query.exec(sql)) {
    fail(query.lastError());
  }
}

void execute(QSqlQuery& query) {
  if (!query.exec()) {
    fail(query.lastError());
  }
}

void prepare(QSqlQuery& query, const QString& sql) {
  if (!query.prepare(sql)) {
    fail(query.lastError());
  }
}

class Transaction final {
public:
  explicit Transaction(QSqlDatabase& database) : database_(database) {
    if (!database_.transaction()) {
      fail(database_.lastError());
    }
  }
  ~Transaction() {
    if (!committed_) {
      database_.rollback();
    }
  }
  Transaction(const Transaction&) = delete;
  Transaction& operator=(const Transaction&) = delete;
  void commit() {
    if (!database_.commit()) {
      fail(database_.lastError());
    }
    committed_ = true;
  }

private:
  QSqlDatabase& database_;
  bool committed_ = false;
};

void bindDraft(QSqlQuery& query, const ProjectDraft& draft) {
  // Empty optional QString values must be TEXT, never SQL NULL.
  query.bindValue(QStringLiteral(":status"), storedStatus(draft.status));
  query.bindValue(QStringLiteral(":photo"), draft.photo.isEmpty()
                                                ? QVariant(QMetaType::fromType<QByteArray>())
                                                : QVariant(draft.photo));
  query.bindValue(QStringLiteral(":name"), draft.name.isNull() ? QStringLiteral("") : draft.name);
  query.bindValue(QStringLiteral(":manufacturer"),
                  draft.manufacturer.isNull() ? QStringLiteral("") : draft.manufacturer);
  query.bindValue(QStringLiteral(":location"),
                  draft.location.isNull() ? QStringLiteral("") : draft.location);
  query.bindValue(QStringLiteral(":note"), draft.note.isNull() ? QStringLiteral("") : draft.note);
}

void requireAffectedRow(const QSqlQuery& query) {
  if (query.numRowsAffected() != 1) {
    throw StorageError(QCoreApplication::translate(
        "App", "Dieses Projekt ist nicht mehr vorhanden. "
               "Lade die Sammlung erneut. Deine Eingaben bleiben erhalten."));
  }
}

} // namespace

SqliteProjectRepository::SqliteProjectRepository(QString directory)
    : directory_(std::move(directory)) {}

SqliteProjectRepository::~SqliteProjectRepository() { close(); }

void SqliteProjectRepository::close() {
  initialized_ = false;
  if (database_.isValid()) {
    database_.close();
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName_);
  }
  lock_.reset();
}

void SqliteProjectRepository::initialize() {
  if (initialized_) {
    return;
  }
  close();
  if (!QDir::isAbsolutePath(directory_) || !QDir().mkpath(directory_)) {
    throw StorageError(
        QCoreApplication::translate("App", "Das Datenverzeichnis kann nicht angelegt werden. "
                                           "Prüfe Pfad, Datenträger und Zugriffsrechte."));
  }
  directory_ = QFileInfo(directory_).canonicalFilePath();
  lock_ = std::make_unique<QLockFile>(QDir(directory_).filePath(QStringLiteral("collection.lock")));
  lock_->setStaleLockTime(0);
  if (!lock_->tryLock()) {
    const bool otherInstance = lock_->error() == QLockFile::LockFailedError;
    lock_.reset();
    throw StorageError(
        otherInstance
            ? QCoreApplication::translate(
                  "App", "Diese Sammlung ist bereits in einer anderen App-Instanz geöffnet. "
                         "Schließe sie dort und lade hier erneut.")
            : QCoreApplication::translate(
                  "App",
                  "Das Datenverzeichnis ist nicht beschreibbar. Prüfe deine Zugriffsrechte."));
  }
  try {
    const QString path = QDir(directory_).filePath(QStringLiteral("modellbauprojekt.sqlite3"));
    const bool isNew = !QFileInfo::exists(path);
    connectionName_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
    database_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName_);
    database_.setDatabaseName(path);
    database_.setConnectOptions(QStringLiteral("QSQLITE_BUSY_TIMEOUT=2000"));
    if (!database_.open()) {
      fail(database_.lastError());
    }
    QSqlQuery query(database_);
    execute(query, QStringLiteral("PRAGMA user_version"));
    if (!query.next()) {
      throw StorageError(
          QCoreApplication::translate("App", "Die Schema-Version konnte nicht gelesen werden."));
    }
    const int version = query.value(0).toInt();
    query.finish();
    if (version > 1) {
      throw StorageError(QCoreApplication::translate(
          "App", "Diese Datenbank benötigt eine neuere Version von ModellbauProjekt. "
                 "Die vorhandene Datei wird nicht verändert."));
    }
    if (isNew) {
      Transaction transaction(database_);
      execute(query, schemaSql());
      execute(query, QStringLiteral("PRAGMA application_id=1296191537"));
      execute(query, QStringLiteral("PRAGMA user_version=1"));
      query.finish();
      transaction.commit();
    } else if (version != 1) {
      throw StorageError(QCoreApplication::translate(
          "App", "Die vorhandene Datei hat kein bekanntes ModellbauProjekt-Schema. "
                 "Sie wird nicht überschrieben."));
    }
    execute(query, QStringLiteral("PRAGMA application_id"));
    if (!query.next() || query.value(0).toInt() != 1296191537) {
      throw StorageError(QCoreApplication::translate(
          "App",
          "Diese Datenbank gehört nicht zu ModellbauProjekt. Die Datei wird nicht ersetzt."));
    }
    query.finish();
    execute(query,
            QStringLiteral("SELECT sql FROM sqlite_master WHERE type='table' AND name='projects'"));
    if (!query.next() || query.value(0).toString() != schemaSql()) {
      throw StorageError(QCoreApplication::translate(
          "App",
          "Die Datenbankstruktur ist unbekannt oder beschädigt. Die Datei wird nicht ersetzt."));
    }
    query.finish();
    execute(query, QStringLiteral("SELECT status FROM projects WHERE status IS NULL OR status NOT "
                                  "IN ('planned','started','finished') LIMIT 1"));
    if (query.next()) {
      (void)readStatus(query.value(0));
    }
    if (query.lastError().isValid())
      fail(query.lastError());
    query.finish();
    execute(query, QStringLiteral("PRAGMA quick_check"));
    if (!query.next() || query.value(0).toString() != QStringLiteral("ok")) {
      throw StorageError(
          QCoreApplication::translate("App", "Die Datenbankprüfung hat einen Schaden gefunden. "
                                             "Sichere die Datei. Sie wird nicht ersetzt."));
    }
    query.finish();
    initialized_ = true;
  } catch (...) {
    close();
    throw;
  }
}

ProjectList SqliteProjectRepository::findAll() {
  QSqlQuery query(database_);
  execute(query, QStringLiteral("SELECT id,name,manufacturer,location,note,status,photo IS NOT "
                                "NULL FROM projects ORDER BY id"));
  ProjectList projects;
  while (query.next()) {
    projects.append({query.value(0).toLongLong(),
                     {query.value(1).toString(),
                      query.value(2).toString(),
                      query.value(3).toString(),
                      query.value(4).toString(),
                      readStatus(query.value(5)),
                      {}},
                     query.value(6).toBool()});
  }
  if (query.lastError().isValid()) {
    fail(query.lastError());
  }
  return projects;
}

ProjectDraft SqliteProjectRepository::find(qint64 id) {
  QSqlQuery query(database_);
  // The bounded expression avoids materializing a corrupted oversized BLOB.
  prepare(query, QStringLiteral(
                     "SELECT name,manufacturer,location,note,status,"
                     "CASE WHEN length(photo)<=10485760 THEN photo ELSE NULL END,"
                     "photo IS NOT NULL,length(photo),typeof(photo) FROM projects WHERE id=:id"));
  query.bindValue(QStringLiteral(":id"), id);
  execute(query);
  if (!query.next()) {
    if (query.lastError().isValid())
      fail(query.lastError());
    throw StorageError(QCoreApplication::translate(
        "App", "Dieses Projekt ist nicht mehr vorhanden. Bitte lade die Liste erneut."));
  }
  if (query.value(6).toBool() &&
      (query.value(7).toLongLong() < 1 || query.value(7).toLongLong() > maxPhotoBytes ||
       query.value(8).toString() != QStringLiteral("blob"))) {
    throw StorageError(QCoreApplication::translate(
        "App", "Das gespeicherte Foto ist ungültig. Der Bearbeitungsdialog "
               "wird zum Schutz der Daten nicht geöffnet."));
  }
  return {query.value(0).toString(), query.value(1).toString(),  query.value(2).toString(),
          query.value(3).toString(), readStatus(query.value(4)), query.value(5).toByteArray()};
}

ModelProject SqliteProjectRepository::create(const ProjectDraft& draft) {
  const auto valid = validateDraft(draft);
  if (!valid.photo.isEmpty()) {
    (void)decodePhoto(valid.photo);
  }
  Transaction transaction(database_);
  QSqlQuery query(database_);
  prepare(query,
          QStringLiteral("INSERT INTO projects(name,manufacturer,location,note,status,photo) "
                         "VALUES(:name,:manufacturer,:location,:note,:status,:photo)"));
  bindDraft(query, valid);
  execute(query);
  const auto id = query.lastInsertId().toLongLong();
  query.finish();
  transaction.commit();
  return summarize(id, valid);
}

ModelProject SqliteProjectRepository::update(qint64 id, const ProjectDraft& draft) {
  const auto valid = validateDraft(draft);
  if (!valid.photo.isEmpty()) {
    (void)decodePhoto(valid.photo);
  }
  Transaction transaction(database_);
  QSqlQuery query(database_);
  prepare(query, QStringLiteral(
                     "UPDATE projects SET name=:name,manufacturer=:manufacturer,location=:location,"
                     "note=:note,status=:status,photo=:photo WHERE id=:id"));
  bindDraft(query, valid);
  query.bindValue(QStringLiteral(":id"), id);
  execute(query);
  requireAffectedRow(query);
  query.finish();
  transaction.commit();
  return summarize(id, valid);
}

void SqliteProjectRepository::remove(qint64 id) {
  Transaction transaction(database_);
  QSqlQuery query(database_);
  prepare(query, QStringLiteral("DELETE FROM projects WHERE id=:id"));
  query.bindValue(QStringLiteral(":id"), id);
  execute(query);
  requireAffectedRow(query);
  query.finish();
  transaction.commit();
}

} // namespace modelbuild
