#include "impl/session.hpp"

#include "core/compilation/diagnostic.hpp"
#include "core/compilation/line.hpp"
#include "core/utility/match.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/common/string.hpp"
#include "impl/session/pass_state.hpp"

#include <mewo/mewo.h>

#include <cassert>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

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

uint64_t mewo_session_get_pass_status_revision(mewo_session_t* session) {
  assert(session);
  return session->impl->pass_status_revision();
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
      [](const mewo::impl::PassState::Pending&) { return MEWO_PASS_COMPILE_STATUS_PENDING; },
      [](const mewo::impl::PassState::Success&) { return MEWO_PASS_COMPILE_STATUS_SUCCESS; },
      [](const mewo::impl::PassState::Error&) { return MEWO_PASS_COMPILE_STATUS_ERROR; },
    },
    state->compile_status()
  );

  return MEWO_SUCCESS;
}

mewo_result_t mewo_session_get_pass_diagnostic_count(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  uint32_t* out
) {
  assert(session);
  assert(out);

  const mewo::impl::PassState* state = session->impl->get_state(pass_id);

  if (!state)
    return MEWO_ERROR_INVALID_ARGUMENT;

  *out = static_cast<uint32_t>(state->diagnostics().size());
  return MEWO_SUCCESS;
}

mewo_result_t mewo_session_get_pass_diagnostic(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  uint32_t index,
  mewo_diagnostic_t* out
) {
  assert(session);
  assert(out);

  const mewo::impl::PassState* state = session->impl->get_state(pass_id);

  if (!state)
    return MEWO_ERROR_INVALID_ARGUMENT;

  const std::vector<mewo::Diagnostic>& diagnostics = state->diagnostics();

  if (index >= diagnostics.size())
    return MEWO_ERROR_INVALID_ARGUMENT;

  const mewo::Diagnostic& diagnostic = diagnostics[index];

  *out = mewo_diagnostic_t{
    .severity =
      [&diagnostic] {
        switch (diagnostic.severity) {
          case mewo::Severity::kError: return MEWO_DIAGNOSTIC_SEVERITY_ERROR;
          case mewo::Severity::kWarning: return MEWO_DIAGNOSTIC_SEVERITY_WARNING;
          case mewo::Severity::kInfo: return MEWO_DIAGNOSTIC_SEVERITY_INFO;
        }
        std::unreachable();
      }(),
    .message = mewo::impl::to_string_view(diagnostic.message),
    .source = MEWO_DIAGNOSTIC_SOURCE_NONE,
    .line = 0,
    .column = 0,
  };

  if (diagnostic.location) {
    out->source = [&diagnostic] {
      switch (diagnostic.location->line.kind) {
        case mewo::RelativeLine::Kind::kPass: return MEWO_DIAGNOSTIC_SOURCE_PASS;
        case mewo::RelativeLine::Kind::kCommon: return MEWO_DIAGNOSTIC_SOURCE_COMMON;
        case mewo::RelativeLine::Kind::kPrelude: return MEWO_DIAGNOSTIC_SOURCE_GENERATED;
      }
      std::unreachable();
    }();
    out->line = diagnostic.location->line.number;
    out->column = diagnostic.location->column;
  }

  return MEWO_SUCCESS;
}
}
