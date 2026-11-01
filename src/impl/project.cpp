#include "impl/project.hpp"

#include "impl/project/pass.hpp"

#include <memory>
#include <vector>

namespace mewo::impl {

std::unique_ptr<Project> Project::create() {
  std::vector<Pass> passes;
  passes.push_back(Pass("Pass 1"));

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Project>(new Project(std::move(passes)));
}

}  // namespace mewo::impl
