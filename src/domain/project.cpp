#include "domain/project.h"
#include <QCoreApplication>

#include <QCollator>
#include <QLocale>
#include <QRegularExpression>
#include <algorithm>

namespace modelbuild {

QString storedStatus(ProjectStatus status) {
  switch (status) {
  case ProjectStatus::Planned:
    return QStringLiteral("planned");
  case ProjectStatus::Started:
    return QStringLiteral("started");
  case ProjectStatus::Finished:
    return QStringLiteral("finished");
  }
  throw ValidationError(QCoreApplication::translate("App", "Der Projektstatus ist ungültig."));
}
QString displayStatus(ProjectStatus status) {
  switch (status) {
  case ProjectStatus::Planned:
    return QCoreApplication::translate("App", "Geplant");
  case ProjectStatus::Started:
    return QCoreApplication::translate("App", "Angefangen");
  case ProjectStatus::Finished:
    return QCoreApplication::translate("App", "Fertig");
  }
  throw ValidationError(QCoreApplication::translate("App", "Der Projektstatus ist ungültig."));
}
ProjectStatus statusFromStorage(const QString& value) {
  if (value == QStringLiteral("planned"))
    return ProjectStatus::Planned;
  if (value == QStringLiteral("started"))
    return ProjectStatus::Started;
  if (value == QStringLiteral("finished"))
    return ProjectStatus::Finished;
  throw ValidationError(QCoreApplication::translate("App",
                                                    "Unbekannter gespeicherter Projektstatus „%1“. "
                                                    "Die Daten werden nicht verändert.")
                            .arg(value));
}
ModelProject summarize(qint64 id, ProjectDraft draft) {
  const bool hasPhoto = !draft.photo.isEmpty();
  draft.photo.clear();
  return {id, std::move(draft), hasPhoto};
}

ProjectDraft validateDraft(ProjectDraft draft) {
  static const QRegularExpression colorPattern(
      QStringLiteral("\\A#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?\\z"));
  if (!draft.color.isEmpty() && !colorPattern.match(draft.color).hasMatch())
    throw ValidationError(QCoreApplication::translate(
        "App", "Die Farbe muss ein Hexwert wie #RRGGBB oder #RRGGBBAA sein."));
  (void)storedStatus(draft.status);
  draft.name = draft.name.trimmed();
  draft.manufacturer = draft.manufacturer.trimmed();
  draft.location = draft.location.trimmed();
  if (draft.name.isEmpty()) {
    throw ValidationError(QCoreApplication::translate("App", "Bitte gib einen Projektnamen ein."));
  }
  return draft;
}

QString searchKey(const QString& text) {
  return text.normalized(QString::NormalizationForm_C)
      .toCaseFolded()
      .normalized(QString::NormalizationForm_C);
}

ProjectList searchProjects(const ProjectList& projects, const QString& query, SortField field,
                           Qt::SortOrder order) {
  const auto needle = searchKey(query.trimmed());
  ProjectList matches;
  for (const auto& project : projects) {
    const auto& f = project.fields;
    if (searchKey(f.name).contains(needle) || searchKey(f.manufacturer).contains(needle) ||
        searchKey(f.location).contains(needle) || searchKey(f.note).contains(needle) ||
        searchKey(f.color).contains(needle)) {
      matches.append(project);
    }
  }
  QCollator collator{QLocale()};
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  collator.setNumericMode(true);
  std::sort(
      matches.begin(), matches.end(),
      [&collator, field, order](const ModelProject& left, const ModelProject& right) {
        int comparison = 0;
        switch (field) {
        case SortField::Name:
          comparison = collator.compare(left.fields.name.normalized(QString::NormalizationForm_C),
                                        right.fields.name.normalized(QString::NormalizationForm_C));
          break;
        case SortField::Details:
          comparison =
              collator.compare(left.fields.manufacturer.normalized(QString::NormalizationForm_C),
                               right.fields.manufacturer.normalized(QString::NormalizationForm_C));
          break;
        case SortField::Color:
          comparison = collator.compare(left.fields.color, right.fields.color);
          break;
        case SortField::Location:
          comparison =
              collator.compare(left.fields.location.normalized(QString::NormalizationForm_C),
                               right.fields.location.normalized(QString::NormalizationForm_C));
          break;
        case SortField::Status:
          comparison = static_cast<int>(left.fields.status) - static_cast<int>(right.fields.status);
          break;
        case SortField::Photo:
          comparison = static_cast<int>(left.hasPhoto) - static_cast<int>(right.hasPhoto);
          break;
        }
        return comparison == 0 ? left.id < right.id
                               : (order == Qt::AscendingOrder ? comparison < 0 : comparison > 0);
      });
  return matches;
}

} // namespace modelbuild
