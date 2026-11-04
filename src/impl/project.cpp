#include "impl/project.hpp"

#include "core/pass.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mewo::impl {
namespace {

constexpr std::string_view kDefaultPassSource = R"(
fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}
)";

}  // namespace

std::unique_ptr<Project> Project::create() {
  // Constructor is private so can't use `std::make_unique`
  std::unique_ptr<Project> project(new Project());
  project->add_pass("Pass 1", std::string(kDefaultPassSource));
  return project;
}

void Project::add_pass(std::string name, std::string source) {
  assert(next_pass_id_ != std::numeric_limits<Pass::Id>::max() && "pass IDs exhausted");
  passes_.emplace_back(next_pass_id_++, std::move(name), std::move(source));
}

bool Project::set_pass_source(Pass::Id pass_id, std::string new_source) {
  if (auto pass_it = std::ranges::find(passes_, pass_id, &Pass::id); pass_it != passes_.end()) {
    pass_it->set_source(std::move(new_source));
    return true;
  }

  return false;
}

void Project::set_common_source(std::string new_source) {
  common_source_ = std::move(new_source);
  common_revision_ += 1;
}

}  // namespace mewo::impl
