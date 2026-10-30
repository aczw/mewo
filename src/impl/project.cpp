#include "impl/project.hpp"

#include <memory>

namespace mewo::impl {

std::unique_ptr<Project> Project::create() {
  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Project>(new Project());
}

}  // namespace mewo::impl
