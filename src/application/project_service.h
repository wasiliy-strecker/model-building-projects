#pragma once

#include "application/project_repository.h"

namespace modelbuild {

class ProjectService final {
public:
  explicit ProjectService(ProjectRepository& repository) : repository_(repository) {}
  ProjectList load();
  ModelProject save(qint64 id, const ProjectDraft& draft);
  void remove(qint64 id);

private:
  ProjectRepository& repository_;
};

} // namespace modelbuild
