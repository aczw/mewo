#pragma once

#include <memory>

namespace mewo::impl {

class Project {
 public:
  static std::unique_ptr<Project> create();

  Project(Project&&) = default;
  Project& operator=(Project&&) = default;
  Project(const Project&) = delete;
  Project& operator=(const Project&) = delete;

 private:
  Project() = default;
};

}  // namespace mewo::impl

struct mewo_project {
  std::unique_ptr<mewo::impl::Project> impl;
};
