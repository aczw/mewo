#include "impl/runtime.hpp"

#include "core/event.hpp"
#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"

#include <webgpu/webgpu.h>

#include <optional>
#include <string_view>

namespace mewo::impl {
namespace {

constexpr std::string_view kUnknownReason = "Unknown reason";

std::string to_string_or_fallback(WGPUStringView view) {
  return gpu::to_string(view).value_or(std::string(kUnknownReason));
}

}  // namespace

std::optional<std::unique_ptr<Runtime>> Runtime::create() {
  WGPUInstanceDescriptor instance_desc = WGPU_INSTANCE_DESCRIPTOR_INIT;
  WGPUInstance instance = wgpuCreateInstance(&instance_desc);

  if (!instance)
    return std::nullopt;

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Runtime>(new Runtime(gpu::Instance(instance)));
}

std::optional<Event> Runtime::poll_event() {
  if (event_queue_.empty())
    return std::nullopt;

  Event popped_event = std::move(event_queue_.front());
  event_queue_.pop_front();

  return popped_event;
}

void Runtime::tick() { wgpuInstanceProcessEvents(instance_.get()); }

void Runtime::request_adapter_and_device(const gpu::Surface& surface) {
  WGPURequestAdapterOptions adapter_opts = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
  adapter_opts.featureLevel = WGPUFeatureLevel_Core;
  adapter_opts.powerPreference = WGPUPowerPreference_HighPerformance;
  adapter_opts.compatibleSurface = surface.get();

  WGPURequestAdapterCallbackInfo adapter_cb_info = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
  adapter_cb_info.mode = WGPUCallbackMode_AllowProcessEvents;
  adapter_cb_info.callback = [](
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
        rt.adapter_ = gpu::Adapter(adapter);
        rt.request_device();
        break;
      }

      case WGPURequestAdapterStatus_Unavailable: {
        rt.event_queue_.push_back(
          AdapterRequestFailed{
            .reason = AdapterRequestFailed::Reason::kUnavailable,
            .message = to_string_or_fallback(message),
          }
        );
        break;
      }

      case WGPURequestAdapterStatus_Error: {
        rt.event_queue_.push_back(
          AdapterRequestFailed{
            .reason = AdapterRequestFailed::Reason::kError,
            .message = to_string_or_fallback(message),
          }
        );
        break;
      }

      default: break;
    }
  };
  adapter_cb_info.userdata1 = static_cast<void*>(this);

  wgpuInstanceRequestAdapter(instance_.get(), &adapter_opts, adapter_cb_info);
}

void Runtime::request_device() {
  WGPUDeviceDescriptor device_desc = WGPU_DEVICE_DESCRIPTOR_INIT;
  device_desc.label = gpu::to_string_view("device");
  device_desc.defaultQueue.label = gpu::to_string_view("default-queue");

  WGPUDeviceLostCallbackInfo lost_cb_info = WGPU_DEVICE_LOST_CALLBACK_INFO_INIT;
  lost_cb_info.mode = WGPUCallbackMode_AllowProcessEvents;
  lost_cb_info.callback =
    [](
      const WGPUDevice*, WGPUDeviceLostReason reason, WGPUStringView message, void* userdata1, void*
    ) noexcept {
      if (reason == WGPUDeviceLostReason_CallbackCancelled)
        return;
      // We already have a callback that will run for this reason
      if (reason == WGPUDeviceLostReason_FailedCreation)
        return;

      Runtime& rt = *static_cast<Runtime*>(userdata1);
      rt.event_queue_.push_back(
        DeviceLost{
          .reason = reason,
          .message = to_string_or_fallback(message),
        }
      );
    };
  lost_cb_info.userdata1 = static_cast<void*>(this);
  device_desc.deviceLostCallbackInfo = lost_cb_info;

  WGPUUncapturedErrorCallbackInfo error_cb_info = WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_INIT;
  error_cb_info.callback =
    [](
      const WGPUDevice*, WGPUErrorType type, WGPUStringView message, void* userdata1, void*
    ) noexcept {
      Runtime& rt = *static_cast<Runtime*>(userdata1);
      rt.event_queue_.push_back(
        GpuErrorOccurred{
          .type = type,
          .message = to_string_or_fallback(message),
        }
      );
    };
  error_cb_info.userdata1 = static_cast<void*>(this);
  device_desc.uncapturedErrorCallbackInfo = error_cb_info;

  WGPURequestDeviceCallbackInfo device_cb_info = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
  device_cb_info.mode = WGPUCallbackMode_AllowProcessEvents;
  device_cb_info.callback = [](
                              WGPURequestDeviceStatus status,
                              WGPUDevice device,
                              WGPUStringView message,
                              void* userdata1,
                              void*
                            ) noexcept {
    Runtime& rt = *static_cast<Runtime*>(userdata1);

    switch (status) {
      case WGPURequestDeviceStatus_CallbackCancelled: return;

      case WGPURequestDeviceStatus_Success: {
        rt.device_ = gpu::Device(device);
        rt.event_queue_.push_back(DeviceRequestSucceeded{});
        break;
      }

      case WGPURequestDeviceStatus_Error: {
        rt.event_queue_.push_back(DeviceRequestFailed{.message = to_string_or_fallback(message)});
        break;
      }

      default: break;
    }
  };
  device_cb_info.userdata1 = static_cast<void*>(this);

  wgpuAdapterRequestDevice(adapter_.get(), &device_desc, device_cb_info);
}

}  // namespace mewo::impl
