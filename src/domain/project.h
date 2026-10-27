#pragma once

#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QVector>
#include <stdexcept>

namespace modelbuild {

enum class ProjectStatus : quint8 { Planned, Started, Finished };
QString storedStatus(ProjectStatus status);
QString displayStatus(ProjectStatus status);
ProjectStatus statusFromStorage(const QString& value);

struct ProjectDraft {
  QString name;
  QString manufacturer;
  QString location;
  QString note;
  ProjectStatus status = ProjectStatus::Planned;
  QByteArray photo = {};
  bool operator==(const ProjectDraft&) const = default;
};

struct ModelProject {
  qint64 id = 0;
  ProjectDraft fields;
  bool hasPhoto = false;
  bool operator==(const ModelProject&) const = default;
};

// List summaries never contain photo bytes. Full drafts are loaded by ID.
ModelProject summarize(qint64 id, ProjectDraft draft);

using ProjectList = QVector<ModelProject>;

class ValidationError final : public std::runtime_error {
public:
  explicit ValidationError(const QString& message) : std::runtime_error(message.toStdString()) {}
};

ProjectDraft validateDraft(ProjectDraft draft);
QString searchKey(const QString& text);
ProjectList searchProjects(const ProjectList& projects, const QString& query);

} // namespace modelbuild

Q_DECLARE_METATYPE(modelbuild::ProjectDraft)
Q_DECLARE_METATYPE(modelbuild::ModelProject)
Q_DECLARE_METATYPE(modelbuild::ProjectList)

Q_DECLARE_METATYPE(modelbuild::ProjectStatus)
