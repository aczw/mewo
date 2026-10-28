#include "runtime/runtime.hpp"

#include "gpu/string.hpp"

#include <webgpu/webgpu.h>

namespace mewo {

std::expected<std::unique_ptr<Runtime>, Error> Runtime::create() {
  WGPUInstanceDescriptor instance_desc = WGPU_INSTANCE_DESCRIPTOR_INIT;
  WGPUInstance instance = wgpuCreateInstance(&instance_desc);

  if (!instance)
    return std::unexpected(Error::kInstanceCreationFailed);

  // I would love to use `std::make_unique` here but the constructor is private
  return std::unique_ptr<Runtime>(new Runtime(Instance(instance)));
}

void Runtime::request_device() {
  wgpuInstanceRequestAdapter(
    instance_.get(),
    nullptr,
    WGPURequestAdapterCallbackInfo{
      .mode = WGPUCallbackMode_AllowProcessEvents,
      .callback =
        [](
          WGPURequestAdapterStatus status,
          WGPUAdapter adapter,
          WGPUStringView message,
          void* userdata1,
          void*
        ) noexcept {
          Runtime& rt = *static_cast<Runtime*>(userdata1);

          switch (status) {
            case WGPURequestAdapterStatus_CallbackCancelled: return;

            case WGPURequestAdapterStatus_Success: {
              rt.adapter_ = Adapter(adapter);
              break;
            }

            case WGPURequestAdapterStatus_Unavailable: {
              rt.event_queue_.push_back(
                AdapterRequestFailed{
                  .reason = AdapterRequestFailed::Reason::kUnavailable,
                  .message = to_string(message).value_or("Unavailable"),
                }
              );
              break;
            }

            case WGPURequestAdapterStatus_Error: {
              rt.event_queue_.push_back(
                AdapterRequestFailed{
                  .reason = AdapterRequestFailed::Reason::kError,
                  .message = to_string(message).value_or("Error"),
                }
              );
              break;
            }

            default: break;
          }
        },
      .userdata1 = static_cast<void*>(this),
    }
  );
}

}  // namespace mewo
