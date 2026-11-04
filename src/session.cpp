#include "impl/session.hpp"

#include "core/utility/match.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/common/string.hpp"
#include "impl/runtime/compile_request.hpp"
#include "impl/session/pass_state.hpp"

#include <mewo/mewo.h>

#include <webgpu/webgpu.h>

#include <cassert>
#include <optional>
#include <utility>
#include <variant>

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

uint64_t mewo_session_get_status_revision(mewo_session_t* session) {
  assert(session);
  return session->impl->status_revision();
}

mewo_result_t mewo_session_get_pass_compile_status(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  mewo_pass_compile_status_t* out
) {
  assert(session);
  assert(out);

  mewo::impl::PassState* state = session->impl->get_state(pass_id);

  if (!state)
    return MEWO_ERROR_INVALID_ARGUMENT;

  *out = std::visit(
    mewo::Match{
      [](const mewo::impl::PassState::Pending&) {
        return mewo_pass_compile_status_t{
          .type = MEWO_PASS_COMPILE_STATUS_PENDING,
          .variant = {},
        };
      },

      [](const mewo::impl::PassState::Success&) {
        return mewo_pass_compile_status_t{
          .type = MEWO_PASS_COMPILE_STATUS_SUCCESS,
          .variant = {},
        };
      },

      [](const mewo::impl::CompileError& variant) {
        return mewo_pass_compile_status_t{
          .type = MEWO_PASS_COMPILE_STATUS_ERROR,
          .variant = {
            .compile_error = {
              .type = variant.status == WGPUCreatePipelineAsyncStatus_ValidationError
                        ? MEWO_PASS_COMPILE_ERROR_VALIDATION
                        : MEWO_PASS_COMPILE_ERROR_INTERNAL,
              .message = mewo::impl::to_string_view(variant.message),
            }
          },
        };
      }
    },
    state->compile_status()
  );

  return MEWO_SUCCESS;
}
}
