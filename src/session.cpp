#include "impl/session.hpp"

#include "impl/common/dimensions.hpp"

#include <mewo/mewo.h>

#include <cassert>
#include <optional>
#include <utility>

extern "C" {
mewo_result_t mewo_session_create(
  mewo_runtime_t* runtime,
  const mewo_session_descriptor_t* desc,
  mewo_session_t** out
) {
  assert(runtime);
  assert(desc);
  assert(out);
  *out = nullptr;

  if (!desc->project)
    return MEWO_ERROR_INVALID_ARGUMENT;
  if (desc->render_width == 0 || desc->render_height == 0)
    return MEWO_ERROR_INVALID_ARGUMENT;

  try {
    auto session_opt = mewo::impl::Session::create(
      *runtime->impl,
      *desc->project->impl,
      mewo::impl::Dimensions{.width = desc->render_width, .height = desc->render_height}
    );

    // The only reason this could have failed is because the device is not ready
    if (!session_opt)
      return MEWO_ERROR_NOT_READY;

    *out = new mewo_session_t(std::move(session_opt.value()));
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

void mewo_session_destroy(mewo_session_t* session) { delete session; }

mewo_result_t mewo_session_set_render_size(
  mewo_session_t* session,
  uint32_t new_width,
  uint32_t new_height
) {
  assert(session);

  if (new_width == 0 || new_height == 0)
    return MEWO_ERROR_INVALID_ARGUMENT;

  session->impl->set_render_size(mewo::impl::Dimensions{.width = new_width, .height = new_height});
  return MEWO_SUCCESS;
}

mewo_result_t mewo_session_render(
  mewo_session_t* session,
  mewo_surface_t* surface,
  uint64_t current_time
) {
  assert(session);

  try {
    session->impl->render(surface ? surface->impl.get() : nullptr, current_time);
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}
}
