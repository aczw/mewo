#include "impl/project.hpp"

#include "core/pass.hpp"
#include "core/validation.hpp"
#include "impl/common/string.hpp"

#include <mewo/mewo.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

static_assert(std::is_same_v<mewo_pass_id_t, mewo::Pass::Id>);

namespace mewo {
namespace {

mewo_name_check_t to_name_check(std::optional<NameError> error) {
  if (!error)
    return MEWO_NAME_CHECK_OK;

  switch (error.value()) {
    case NameError::kEmpty: return MEWO_NAME_CHECK_EMPTY;
    case NameError::kTooLong: return MEWO_NAME_CHECK_TOO_LONG;
    case NameError::kInvalidCharacter: return MEWO_NAME_CHECK_INVALID_CHARACTER;
    case NameError::kLeadingOrTrailing: return MEWO_NAME_CHECK_LEADING_OR_TRAILING;
    case NameError::kReserved: return MEWO_NAME_CHECK_RESERVED;
    case NameError::kTaken: return MEWO_NAME_CHECK_TAKEN;
  }

  std::unreachable();
}

}  // namespace
}  // namespace mewo

extern "C" {
mewo_result_t mewo_project_create(const mewo_project_descriptor_t* desc, mewo_project_t** out) {
  assert(desc);
  assert(out);
  *out = nullptr;

  try {
    auto project_name_opt = mewo::impl::to_string(desc->name);
    auto name_opt = mewo::impl::to_string(desc->first_pass_name);
    auto source_opt = mewo::impl::to_string(desc->first_pass_source);

    if (!project_name_opt || !name_opt || !source_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    // An empty name means the default, so zero-initialized descriptors work
    if (project_name_opt->empty())
      project_name_opt = std::nullopt;

    auto project_opt = mewo::impl::Project::create(
      std::move(project_name_opt), std::move(name_opt.value()), std::move(source_opt.value())
    );

    if (!project_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    *out = new mewo_project_t(std::move(project_opt.value()));
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

void mewo_project_destroy(mewo_project_t* project) { delete project; }

mewo_string_view_t mewo_project_get_name(mewo_project_t* project) {
  assert(project);
  return mewo::impl::to_string_view(project->impl->name());
}

mewo_result_t mewo_project_set_name(mewo_project_t* project, mewo_string_view_t name) {
  assert(project);

  // `Project::set_name` copies the name into a `std::string`
  try {
    std::optional<std::string_view> name_opt = mewo::impl::to_std_string_view(name);

    if (!name_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    if (!project->impl->set_name(name_opt.value()))
      return MEWO_ERROR_INVALID_ARGUMENT;
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

mewo_result_t mewo_check_project_name(mewo_string_view_t name, mewo_name_check_t* out) {
  assert(out);
  *out = MEWO_NAME_CHECK_OK;

  std::optional<std::string_view> name_opt = mewo::impl::to_std_string_view(name);

  if (!name_opt)
    return MEWO_ERROR_INVALID_ARGUMENT;

  *out = mewo::to_name_check(mewo::check_name(name_opt.value()));

  return MEWO_SUCCESS;
}

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

mewo_result_t mewo_project_set_pass_name(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  mewo_string_view_t name
) {
  assert(project);

  // We're attempting to allocate a new `std::string`
  try {
    auto name_opt = mewo::impl::to_string(name);

    if (!name_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    if (!project->impl->set_pass_name(pass_id, std::move(name_opt.value())))
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

mewo_result_t mewo_project_check_pass_name(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  mewo_string_view_t name,
  mewo_name_check_t* out
) {
  assert(project);
  assert(out);
  *out = MEWO_NAME_CHECK_OK;

  std::optional<std::string_view> name_opt = mewo::impl::to_std_string_view(name);

  if (!name_opt)
    return MEWO_ERROR_INVALID_ARGUMENT;

  std::optional<mewo::Pass::Id> except;

  if (pass_id != MEWO_PASS_ID_NONE) {
    if (!std::ranges::contains(project->impl->passes(), pass_id, &mewo::Pass::id))
      return MEWO_ERROR_INVALID_ARGUMENT;
    except = pass_id;
  }

  *out = mewo::to_name_check(project->impl->check_pass_name(name_opt.value(), except));

  return MEWO_SUCCESS;
}

mewo_result_t mewo_project_add_pass(
  mewo_project_t* project,
  mewo_string_view_t name,
  mewo_string_view_t source,
  mewo_pass_id_t* out
) {
  assert(project);
  assert(out);
  *out = MEWO_PASS_ID_NONE;

  // Copies both strings and may grow the pass list
  try {
    auto name_opt = mewo::impl::to_string(name);
    auto source_opt = mewo::impl::to_string(source);

    if (!name_opt || !source_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    auto pass_id_opt =
      project->impl->add_pass(std::move(name_opt.value()), std::move(source_opt.value()));

    if (!pass_id_opt)
      return MEWO_ERROR_INVALID_ARGUMENT;

    *out = pass_id_opt.value();
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

mewo_result_t mewo_project_remove_pass(mewo_project_t* project, mewo_pass_id_t pass_id) {
  assert(project);

  switch (project->impl->remove_pass(pass_id)) {
    case mewo::impl::Project::RemovePassResult::kSuccess: return MEWO_SUCCESS;
    case mewo::impl::Project::RemovePassResult::kNotFound: return MEWO_ERROR_INVALID_ARGUMENT;
    case mewo::impl::Project::RemovePassResult::kFailedBecauseLast:
      return MEWO_ERROR_INVALID_OPERATION;
  }

  std::unreachable();
}

mewo_result_t mewo_project_move_pass(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  uint32_t new_index
) {
  assert(project);

  if (!project->impl->move_pass(pass_id, new_index))
    return MEWO_ERROR_INVALID_ARGUMENT;

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

mewo_result_t mewo_project_get_pass_name(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  mewo_string_view_t* out
) {
  assert(project);
  assert(out);
  *out = mewo_string_view_t{.data = nullptr, .length = 0};

  const std::vector<mewo::Pass>& passes = project->impl->passes();
  auto pass_it = std::ranges::find(passes, pass_id, &mewo::Pass::id);

  if (pass_it == passes.end())
    return MEWO_ERROR_INVALID_ARGUMENT;

  const std::string& name = pass_it->name();
  *out = mewo_string_view_t{.data = name.data(), .length = name.size()};

  return MEWO_SUCCESS;
}

mewo_result_t mewo_project_get_pass_source(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  mewo_string_view_t* out
) {
  assert(project);
  assert(out);
  *out = mewo_string_view_t{.data = nullptr, .length = 0};

  const std::vector<mewo::Pass>& passes = project->impl->passes();
  auto pass_it = std::ranges::find(passes, pass_id, &mewo::Pass::id);

  if (pass_it == passes.end())
    return MEWO_ERROR_INVALID_ARGUMENT;

  const std::string& source = pass_it->source();
  *out = mewo_string_view_t{.data = source.data(), .length = source.size()};

  return MEWO_SUCCESS;
}

mewo_string_view_t mewo_project_get_common_source(mewo_project_t* project) {
  assert(project);

  const std::string& source = project->impl->common_source();
  return mewo_string_view_t{.data = source.data(), .length = source.size()};
}
}
