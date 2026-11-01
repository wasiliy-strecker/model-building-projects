#include "persistence/sqlite_project_repository.h"
#include "application/collection_codec.h"
#include "application/photo_importer.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QMap>

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

QString schemaForVersion(int version) {
  const auto original = schemaSql();
  return version == 2 ? original.chopped(1) + QStringLiteral(", color TEXT NOT NULL DEFAULT '')")
                      : original;
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
  query.bindValue(QStringLiteral(":color"),
                  draft.color.isNull() ? QStringLiteral("") : draft.color);
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
    if (version > 2) {
      throw StorageError(QCoreApplication::translate(
          "App", "Diese Datenbank benötigt eine neuere Version von Model Building Projects. "
                 "Die vorhandene Datei wird nicht verändert."));
    }
    if (isNew) {
      Transaction transaction(database_);
      execute(query, schemaSql());
      execute(query, QStringLiteral("PRAGMA application_id=1296191537"));
      execute(query, QStringLiteral("PRAGMA user_version=1"));
      query.finish();
      transaction.commit();
    } else if (version != 1 && version != 2) {
      throw StorageError(QCoreApplication::translate(
          "App", "Die vorhandene Datei hat kein bekanntes Model Building Projects-Schema. "
                 "Sie wird nicht überschrieben."));
    }
    execute(query, QStringLiteral("PRAGMA application_id"));
    if (!query.next() || query.value(0).toInt() != 1296191537) {
      throw StorageError(
          QCoreApplication::translate("App", "Diese Datenbank gehört nicht zu Model Building "
                                             "Projects. Die Datei wird nicht ersetzt."));
    }
    query.finish();
    execute(query,
            QStringLiteral("SELECT sql FROM sqlite_master WHERE type='table' AND name='projects'"));
    if (!query.next() || query.value(0).toString() != schemaForVersion(isNew ? 1 : version)) {
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
    if (isNew || version == 1) {
      // Validate identity and the entire old schema before changing an existing collection.
      // DDL and version change commit together. Failed upgrades leave all data intact.
      Transaction migration(database_);
      execute(query,
              QStringLiteral("ALTER TABLE projects ADD COLUMN color TEXT NOT NULL DEFAULT ''"));
      execute(query, QStringLiteral("PRAGMA user_version=2"));
      query.finish();
      migration.commit();
    }
    initialized_ = true;
  } catch (...) {
    close();
    throw;
  }
}

ProjectList SqliteProjectRepository::findAll() {
  Transaction transaction(database_);
  auto result = findAllInTransaction();
  transaction.commit();
  return result;
}
ProjectList SqliteProjectRepository::findAllInTransaction() {
  QSqlQuery query(database_);
  execute(query, QStringLiteral("SELECT id,name,manufacturer,location,note,status,photo IS NOT "
                                "NULL,color FROM projects ORDER BY id"));
  ProjectList projects;
  while (query.next()) {
    projects.append({query.value(0).toLongLong(),
                     {query.value(1).toString(),
                      query.value(2).toString(),
                      query.value(3).toString(),
                      query.value(4).toString(),
                      readStatus(query.value(5)),
                      {},
                      query.value(7).toString()},
                     query.value(6).toBool()});
  }
  if (query.lastError().isValid()) {
    fail(query.lastError());
  }
  return projects;
}

ProjectDraft SqliteProjectRepository::find(qint64 id) {
  Transaction transaction(database_);
  auto result = findInTransaction(id);
  transaction.commit();
  return result;
}
ProjectDraft SqliteProjectRepository::findInTransaction(qint64 id) {
  QSqlQuery query(database_);
  // The bounded expression avoids materializing a corrupted oversized BLOB.
  prepare(query,
          QStringLiteral(
              "SELECT name,manufacturer,location,note,status,"
              "CASE WHEN length(photo)<=10485760 THEN photo ELSE NULL END,"
              "photo IS NOT NULL,length(photo),typeof(photo),color FROM projects WHERE id=:id"));
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
          query.value(3).toString(), readStatus(query.value(4)), query.value(5).toByteArray(),
          query.value(9).toString()};
}

ModelProject SqliteProjectRepository::create(const ProjectDraft& draft) {
  Transaction transaction(database_);
  auto result = createInTransaction(draft);
  transaction.commit();
  return result;
}
ModelProject SqliteProjectRepository::createInTransaction(const ProjectDraft& draft) {
  const auto valid = validateDraft(draft);
  if (!valid.photo.isEmpty()) {
    (void)decodePhoto(valid.photo);
  }
  QSqlQuery query(database_);
  prepare(query,
          QStringLiteral("INSERT INTO projects(name,manufacturer,location,note,status,photo,color) "
                         "VALUES(:name,:manufacturer,:location,:note,:status,:photo,:color)"));
  bindDraft(query, valid);
  execute(query);
  const auto id = query.lastInsertId().toLongLong();
  query.finish();
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
                     "note=:note,status=:status,photo=:photo,color=:color WHERE id=:id"));
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

TransferDraftList SqliteProjectRepository::transferSnapshot() {
  Transaction transaction(database_);
  const auto rows = findAllInTransaction();
  if (rows.size() > maxTransferEntries)
    throw TransferError(QCoreApplication::translate(
        "App", "Eine Sammlung darf höchstens 10.000 Einträge und 256 MiB enthalten."));
  TransferDraftList result;
  qint64 size = 0;
  for (const auto& row : rows) {
    auto draft = findInTransaction(row.id);
    size += QJsonDocument(encodeEntry(draft)).toJson(QJsonDocument::Compact).size() + 8;
    if (size > maxTransferBytes)
      throw TransferError(QCoreApplication::translate(
          "App", "Eine Sammlung darf höchstens 10.000 Einträge und 256 MiB enthalten."));
    result.append(std::move(draft));
  }
  transaction.commit();
  return result;
}
CollectionImportResult SqliteProjectRepository::importEntries(const TransferDraftList& drafts) {
  if (drafts.size() > maxTransferEntries)
    throw TransferError(QCoreApplication::translate(
        "App", "Eine Sammlung darf höchstens 10.000 Einträge und 256 MiB enthalten."));
  qint64 size = 0;
  for (const auto& draft : drafts) {
    size += QJsonDocument(encodeEntry(draft)).toJson(QJsonDocument::Compact).size() + 8;
    if (size > maxTransferBytes)
      throw TransferError(QCoreApplication::translate(
          "App", "Eine Sammlung darf höchstens 10.000 Einträge und 256 MiB enthalten."));
  }
  Transaction transaction(database_);
  const auto existing = findAllInTransaction();
  QMap<QByteArray, qsizetype> available;
  for (const auto& row : existing)
    ++available[entryFingerprint(findInTransaction(row.id))];
  CollectionImportResult result;
  for (const auto& draft : drafts) {
    auto& copies = available[entryFingerprint(draft)];
    if (copies > 0) {
      --copies;
      ++result.skipped;
      continue;
    }
    if (existing.size() + result.added >= maxTransferEntries)
      throw TransferError(
          QCoreApplication::translate("App", "Der Import würde die Grenze von 10.000 Einträgen "
                                             "überschreiten. Die Sammlung bleibt unverändert."));
    (void)createInTransaction(draft);
    ++result.added;
  }
  result.entries = findAllInTransaction();
  transaction.commit();
  return result;
}

} // namespace modelbuild
