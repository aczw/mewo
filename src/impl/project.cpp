#include "impl/project.hpp"

#include "core/pass.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace mewo::impl {

std::unique_ptr<Project> Project::create(
  std::string&& first_pass_name,
  std::string&& first_pass_source
) {
  // Constructor is private so can't use `std::make_unique`
  std::unique_ptr<Project> project(new Project());
  project->add_pass(std::move(first_pass_name), std::move(first_pass_source));
  return project;
}

Pass::Id Project::add_pass(std::string name, std::string source) {
  assert(next_pass_id_ != std::numeric_limits<Pass::Id>::max() && "pass IDs exhausted");

  Pass::Id next_pass_id = next_pass_id_++;
  passes_.emplace_back(next_pass_id, std::move(name), std::move(source));
  structure_revision_ += 1;

  return next_pass_id;
}

Project::RemovePassResult Project::remove_pass(Pass::Id pass_id) {
  assert(passes_.size() >= 1 && "project does not have at least 1 pass");

  auto pass_it = std::ranges::find(passes_, pass_id, &Pass::id);

  if (pass_it == passes_.end())
    return RemovePassResult::kNotFound;

  if (passes_.size() == 1)
    return RemovePassResult::kFailedBecauseLast;

  passes_.erase(pass_it);
  structure_revision_ += 1;

  return RemovePassResult::kSuccess;
}

bool Project::move_pass(Pass::Id pass_id, uint32_t new_index) {
  assert(passes_.size() >= 1 && "project does not have at least 1 pass");

  if (new_index >= passes_.size())
    return false;

  auto current_it = std::ranges::find(passes_, pass_id, &Pass::id);

  if (current_it == passes_.end())
    return false;

  auto new_it = passes_.begin() + static_cast<std::ptrdiff_t>(new_index);

  if (new_it == current_it)
    return true;

  if (new_it < current_it) {
    std::rotate(new_it, current_it, current_it + 1);
  } else {
    std::rotate(current_it, current_it + 1, new_it + 1);
  }

  structure_revision_ += 1;

  return true;
}

bool Project::set_pass_source(Pass::Id pass_id, std::string new_source) {
  assert(passes_.size() >= 1 && "project does not have at least 1 pass");

  if (auto pass_it = std::ranges::find(passes_, pass_id, &Pass::id); pass_it != passes_.end()) {
    pass_it->set_source(std::move(new_source));
    return true;
  }

  return false;
}

bool Project::set_pass_name(Pass::Id pass_id, std::string new_name) {
  if (auto pass_it = std::ranges::find(passes_, pass_id, &Pass::id); pass_it != passes_.end()) {
    pass_it->set_name(std::move(new_name));
    return true;
  }

  return false;
}

void Project::set_common_source(std::string new_source) {
  assert(passes_.size() >= 1 && "project does not have at least 1 pass");

  common_source_ = std::move(new_source);
  common_revision_ += 1;
}

}  // namespace mewo::impl
