#include "impl/project.hpp"

#include "core/pass.hpp"
#include "impl/common/string.hpp"

#include <mewo/mewo.h>

#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

static_assert(std::is_same_v<mewo_pass_id_t, mewo::Pass::Id>);

extern "C" {
mewo_result_t mewo_project_create(mewo_project_t** out) {
  assert(out);
  *out = nullptr;

  try {
    *out = new mewo_project_t(mewo::impl::Project::create());
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

void mewo_project_destroy(mewo_project_t* project) { delete project; }

mewo_result_t mewo_project_set_pass_source(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  mewo_string_view_t source
) {
  assert(project);

  // We're attempting to allocate a new `std::string`
  try {
    auto code_opt = mewo::impl::to_string(source);

    if (!code_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    if (!project->impl->set_pass_source(pass_id, std::move(code_opt.value())))
      return MEWO_ERROR_INVALID_ARGUMENT;
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

mewo_result_t mewo_project_set_common_source(mewo_project_t* project, mewo_string_view_t source) {
  assert(project);

  // We're attempting to allocate a new `std::string`
  try {
    auto code_opt = mewo::impl::to_string(source);

    if (!code_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    project->impl->set_common_source(std::move(code_opt.value()));
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

uint32_t mewo_project_get_pass_count(mewo_project_t* project) {
  assert(project);

  size_t pass_count = project->impl->passes().size();
  assert(pass_count <= std::numeric_limits<uint32_t>::max());

  // Cast to match the type of `index` in `mewo_project_get_pass_id()`
  return static_cast<uint32_t>(pass_count);
}

mewo_pass_id_t mewo_project_get_pass_id(mewo_project_t* project, uint32_t index) {
  assert(project);

  const std::vector<mewo::Pass>& passes = project->impl->passes();

  if (index >= passes.size())
    return MEWO_PASS_ID_NONE;

  return passes[index].id();
}
}
