#include "impl/runtime.hpp"

#include "core/compilation/diagnostic.hpp"
#include "core/event.hpp"
#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "impl/runtime/compile_request.hpp"

#include <webgpu/webgpu.h>

#include <cstdint>
#include <expected>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace mewo::impl {
namespace {

std::string to_string_or_fallback(WGPUStringView view) {
  return gpu::to_string(view).value_or("invalid message");
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

void Runtime::request_gfx_components(const gpu::Surface* compatible_surface) {
  WGPURequestAdapterOptions adapter_opts = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
  adapter_opts.featureLevel = WGPUFeatureLevel_Core;
  adapter_opts.powerPreference = WGPUPowerPreference_HighPerformance;
  adapter_opts.compatibleSurface = compatible_surface ? compatible_surface->get() : nullptr;

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
        rt.request_device_and_queue();
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

CompileRequestId Runtime::start_compile_request(
  const WGPURenderPipelineDescriptor& render_pipeline_desc,
  const gpu::ShaderModule& shader_module,
  SegmentMap segments
) {
  CompileRequestId request_id = next_compile_request_id_++;
  compile_requests_.try_emplace(request_id, CompileEntry{.segments = segments});

  {
    WGPUCreateRenderPipelineAsyncCallbackInfo cb_info =
      WGPU_CREATE_RENDER_PIPELINE_ASYNC_CALLBACK_INFO_INIT;
    cb_info.mode = WGPUCallbackMode_AllowProcessEvents;
    cb_info.callback = [](
                         WGPUCreatePipelineAsyncStatus status,
                         WGPURenderPipeline pipeline,
                         WGPUStringView message,
                         void* userdata1,
                         void* userdata2
                       ) noexcept {
      if (status == WGPUCreatePipelineAsyncStatus_CallbackCancelled)
        return;

      gpu::RenderPipeline owned_pipeline(pipeline);

      Runtime* runtime = static_cast<Runtime*>(userdata1);
      CompileRequestId request_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(userdata2));

      auto request_it = runtime->compile_requests_.find(request_id);
      if (request_it == runtime->compile_requests_.end())
        return;

      CompileEntry& entry = request_it->second;

      switch (status) {
        case WGPUCreatePipelineAsyncStatus_Success: {
          entry.pipeline_result = std::move(owned_pipeline);
          break;
        }

        default: {
          entry.pipeline_result = std::unexpected(
            PipelineError{.message = gpu::to_string(message).value_or("invalid message")}
          );
          break;
        }
      }
    };
    cb_info.userdata1 = static_cast<void*>(this);
    cb_info.userdata2 = reinterpret_cast<void*>(static_cast<uintptr_t>(request_id));

    wgpuDeviceCreateRenderPipelineAsync(device_.get(), &render_pipeline_desc, cb_info);
  }

  {
    WGPUCompilationInfoCallbackInfo cb_info = WGPU_COMPILATION_INFO_CALLBACK_INFO_INIT;
    cb_info.mode = WGPUCallbackMode_AllowProcessEvents;
    cb_info.callback = [](
                         WGPUCompilationInfoRequestStatus status,
                         struct WGPUCompilationInfo const* compilationInfo,
                         void* userdata1,
                         void* userdata2
                       ) noexcept {
      if (status == WGPUCompilationInfoRequestStatus_CallbackCancelled)
        return;

      Runtime* runtime = static_cast<Runtime*>(userdata1);
      CompileRequestId request_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(userdata2));

      auto request_it = runtime->compile_requests_.find(request_id);
      if (request_it == runtime->compile_requests_.end())
        return;

      CompileEntry& entry = request_it->second;
      std::vector<CompileMessage> messages;

      for (size_t idx = 0; idx < compilationInfo->messageCount; ++idx) {
        const WGPUCompilationMessage& message = compilationInfo->messages[idx];

        messages.push_back(
          CompileMessage{
            .severity = [&message]() -> Severity {
              switch (message.type) {
                case WGPUCompilationMessageType_Error: return Severity::kError;
                case WGPUCompilationMessageType_Warning: return Severity::kWarning;
                case WGPUCompilationMessageType_Info: return Severity::kInfo;
                default: return Severity::kError;
              }
            }(),
            .message = gpu::to_string(message.message).value_or("invalid message"),
            .line_number = message.lineNum,
            .line_column = message.linePos,
          }
        );
      }

      entry.messages = std::move(messages);
    };
    cb_info.userdata1 = static_cast<void*>(this);
    cb_info.userdata2 = reinterpret_cast<void*>(static_cast<uintptr_t>(request_id));

    wgpuShaderModuleGetCompilationInfo(shader_module.get(), cb_info);
  }

  return request_id;
}

std::optional<CompileEntry> Runtime::take_compile_request(CompileRequestId request_id) {
  auto request_it = compile_requests_.find(request_id);

  if (request_it == compile_requests_.end())
    return std::nullopt;

  CompileEntry& entry = request_it->second;

  if (!entry.pipeline_result || !entry.messages)
    return std::nullopt;

  auto request_nh = compile_requests_.extract(request_it);
  return std::optional(std::move(request_nh.mapped()));
}

void Runtime::request_device_and_queue() {
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
        // Queue is created at the same time as the device, so it must exist
        rt.queue_ = gpu::Queue(wgpuDeviceGetQueue(rt.device_.get()));
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
