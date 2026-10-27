#include "persistence/data_directory.h"
#include "persistence/sqlite_project_repository.h"
#include "ui/main_window.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral("de.appfabrik.modellbauprojekt"));
  QCoreApplication::setOrganizationDomain(QStringLiteral("appfabrik.de"));
  QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
  QApplication::setApplicationDisplayName(QStringLiteral("ModellbauProjekt"));
  QApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/branding/icon-256.png")));
  QTranslator translator;
  if (translator.load(QStringLiteral("qtbase_de"),
                      QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
    application.installTranslator(&translator);
  }
  QCommandLineParser parser;
  parser.setApplicationDescription(
      QStringLiteral("Eine lokale Sammlung für deine Modellbauprojekte."));
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption dataOption(QStringLiteral("data-dir"),
                                QStringLiteral("Isoliertes absolutes Datenverzeichnis."),
                                QStringLiteral("pfad"));
  parser.addOption(dataOption);
  parser.process(application);
  try {
    const auto directory =
        modelbuild::resolveDataDirectory(parser.value(dataOption), parser.isSet(dataOption));
    modelbuild::StorageController controller(
        [directory] { return std::make_unique<modelbuild::SqliteProjectRepository>(directory); });
    modelbuild::MainWindow window(controller, directory);
    window.show();
    return QApplication::exec();
  } catch (const std::exception& error) {
    QMessageBox::critical(nullptr, QStringLiteral("ModellbauProjekt kann nicht starten"),
                          QString::fromUtf8(error.what()));
    return 1;
  }
}
