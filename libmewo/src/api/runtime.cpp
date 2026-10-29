#include "runtime/runtime.hpp"

#include "api/handles.hpp"
#include "api/result.hpp"
#include "runtime/event.hpp"
#include "utility/match.hpp"

#include <mewo/mewo.h>

#include <cassert>
#include <utility>
#include <variant>

static_assert(sizeof(mewo_event_t::variant) == 32, "event payload outgrew reserved space");

extern "C" {
mewo_result_t mewo_runtime_create(mewo_runtime_t** out) {
  assert(out);
  *out = nullptr;

  try {
    auto runtime_res = mewo::Runtime::create();

    if (!runtime_res)
      return mewo::error_to_result(runtime_res.error());

    *out = new mewo_runtime(std::move(runtime_res.value()));
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

void mewo_runtime_destroy(mewo_runtime_t* runtime) { delete runtime; }

bool mewo_runtime_poll_event(mewo_runtime_t* runtime, mewo_event_t* out) {
  assert(runtime);
  assert(out);

  auto popped_opt = runtime->impl->poll_event();

  if (!popped_opt) {
    runtime->current_message = "";
    return false;
  }

  const mewo::Event& popped_event = popped_opt.value();
  runtime->current_message = mewo::get_event_message(popped_event);

  auto convert_to_view = [&runtime]() -> mewo_string_view_t {
    return {
      .data = runtime->current_message.data(),
      .length = runtime->current_message.length(),
    };
  };

  *out = std::visit(
    mewo::Match{
      [&convert_to_view](const mewo::AdapterRequestFailed& variant) {
        return mewo_event_t{
          .type = MEWO_EVENT_DEVICE_FAILED,
          .variant = {
            .device_failed = {
              .reason = [&variant]() -> mewo_device_failure_t {
                switch (variant.reason) {
                  case mewo::AdapterRequestFailed::Reason::kError: return MEWO_DEVICE_FAILURE_ERROR;
                  case mewo::AdapterRequestFailed::Reason::kUnavailable:
                    return MEWO_DEVICE_FAILURE_UNAVAILABLE;
                }
                std::unreachable();
              }(),
              .message = convert_to_view(),
            }
          }
        };
      },

      [&convert_to_view](const mewo::DeviceLost&) {
        return mewo_event_t{
          .type = MEWO_EVENT_DEVICE_LOST,
          .variant = {
            .device_lost = {.message = convert_to_view()},
          }
        };
      },

      [&convert_to_view](const mewo::GpuErrorOccurred& variant) {
        return mewo_event_t{
          .type = MEWO_EVENT_GPU_ERROR,
          .variant = {
            .gpu_error = {
              .type = [&variant]() -> mewo_gpu_error_t {
                switch (variant.type) {
                  case WGPUErrorType_Validation: return MEWO_GPU_ERROR_VALIDATION;
                  case WGPUErrorType_OutOfMemory: return MEWO_GPU_ERROR_OUT_OF_MEMORY;
                  case WGPUErrorType_Internal: return MEWO_GPU_ERROR_INTERNAL;
                  default: return MEWO_GPU_ERROR_UNKNOWN;
                }
              }(),
              .message = convert_to_view(),
            }
          },
        };
      },

      [](const mewo::DeviceRequestSucceeded&) {
        return mewo_event_t{.type = MEWO_EVENT_DEVICE_READY, .variant = {}};
      },

      [&convert_to_view](const mewo::DeviceRequestFailed&) {
        return mewo_event_t{
          .type = MEWO_EVENT_DEVICE_FAILED,
          .variant = {
            .device_failed = {
              .reason = MEWO_DEVICE_FAILURE_ERROR,
              .message = convert_to_view(),
            }
          },
        };
      },
    },
    popped_event
  );

  return true;
}

void mewo_runtime_tick(mewo_runtime_t* runtime) {
  assert(runtime);
  runtime->impl->tick();
}

void mewo_runtime_request_device(mewo_runtime_t* runtime) {
  assert(runtime);
  runtime->impl->request_adapter_and_device();
}
}
