#include "renderer.hpp"

#include "exception.hpp"
#include "query.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <functional>
#include <limits>
#include <optional>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#endif

namespace mewo::gfx {

static constexpr uint64_t WGPU_WAIT_TIMEOUT_MAX = std::numeric_limits<uint64_t>::max();

Renderer::Renderer(sdl::Window& window)
{
  auto timed_wait_any = wgpu::InstanceFeatureName::TimedWaitAny;
  wgpu::InstanceDescriptor instance_desc = {
    .requiredFeatureCount = 1,
    .requiredFeatures = &timed_wait_any,
  };

  instance_ = wgpu::CreateInstance(&instance_desc);

  if (!instance_)
    throw Exception("WebGPU instance creation failed");

  wgpu::Adapter adapter = {};
  wgpu::RequestAdapterOptions adapter_opts = {
    .featureLevel = wgpu::FeatureLevel::Core,
    .powerPreference = wgpu::PowerPreference::HighPerformance,
  };

  wgpu::Future request_adapter_future
      = instance_.RequestAdapter(&adapter_opts, wgpu::CallbackMode::WaitAnyOnly,
          [&adapter](wgpu::RequestAdapterStatus status, wgpu::Adapter acquired_adapter,
              wgpu::StringView message) {
            // Throwing here is safe because we wait on callback execution in the current thread
            if (status != wgpu::RequestAdapterStatus::Success)
              throw Exception("Failed to request WebGPU adapter: {}", message.data);

            adapter = std::move(acquired_adapter);
          });
  wgpu::WaitStatus adapter_wait_status
      = instance_.WaitAny(request_adapter_future, WGPU_WAIT_TIMEOUT_MAX);

  if (!adapter || adapter_wait_status != wgpu::WaitStatus::Success)
    throw Exception("Waiting on wgpu::Instance::RequestAdapter failed");

  if constexpr (query::is_debug())
    ImGui_ImplWGPU_DebugPrintAdapterInfo(adapter.Get());

  wgpu::DeviceDescriptor device_desc;
  device_desc.label = "device";
  device_desc.defaultQueue.label = "default-queue";

  device_desc.SetDeviceLostCallback(
      wgpu::CallbackMode::WaitAnyOnly,
      [](const wgpu::Device&, wgpu::DeviceLostReason type, wgpu::StringView message,
          std::optional<Error>* device_lost_error) {
        auto reason = static_cast<WGPUDeviceLostReason>(type);

        *device_lost_error = Error {
          .error_type = ImGui_ImplWGPU_GetDeviceLostReasonName(reason),
          .message = message,
        };
      },
      &device_lost_error_);

  device_desc.SetUncapturedErrorCallback(
      [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message,
          std::optional<Error>* uncaptured_error) {
        auto error_type = static_cast<WGPUErrorType>(type);

        *uncaptured_error = Error {
          .error_type = ImGui_ImplWGPU_GetErrorTypeName(error_type),
          .message = message,
        };
      },
      &uncaptured_error_);

  wgpu::Future request_device_future
      = adapter.RequestDevice(&device_desc, wgpu::CallbackMode::WaitAnyOnly,
          [this](wgpu::RequestDeviceStatus status, wgpu::Device acquired_device,
              wgpu::StringView message) {
            // Throwing here is safe because we wait on callback execution in the current thread
            if (status != wgpu::RequestDeviceStatus::Success)
              throw Exception("Failed to request WebGPU device: {}", message.data);

            device_ = std::move(acquired_device);
          });
  wgpu::WaitStatus device_wait_status
      = instance_.WaitAny(request_device_future, WGPU_WAIT_TIMEOUT_MAX);

  if (!device_ || device_wait_status != wgpu::WaitStatus::Success)
    throw Exception("Waiting on wgpu::Adapter::RequestDevice failed");

  SDL_PropertiesID properties_id = SDL_GetWindowProperties(window.get());

  if (properties_id == 0)
    throw Exception("Failed to get SDL window properties: {}", SDL_GetError());

  ImGui_ImplWGPU_CreateSurfaceInfo create_surface_info = {
    .Instance = instance_.Get(),
#if defined(SDL_PLATFORM_MACOS)
    .System = "cocoa",
    .RawWindow = static_cast<void*>(
        SDL_GetPointerProperty(properties_id, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr)),
#elif defined(SDL_PLATFORM_WIN32)
    .System = "win32",
    .RawWindow = static_cast<void*>(
        SDL_GetPointerProperty(properties_id, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)),
    .RawInstance = static_cast<void*>(GetModuleHandle(nullptr)),
#else
#error "Unsupported platform. Supported platforms are macOS and Windows"
#endif
  };

  if (WGPUSurface raw_surface = ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_surface_info);
      !raw_surface) {
    throw Exception("Failed to create WebGPU surface");
  } else {
    surface_ = wgpu::Surface(raw_surface);
  }

  surface_config_ = std::invoke([this, &window, &adapter] -> wgpu::SurfaceConfiguration {
    wgpu::SurfaceCapabilities surface_capabilities;

    if (!surface_.GetCapabilities(adapter, &surface_capabilities))
      throw Exception("Failed to get WebGPU surface capabilities");
    if (surface_capabilities.formatCount == 0)
      throw Exception("No available WebGPU surface formats");

    int width_in_pixels = 0;
    int height_in_pixels = 0;

    if (!SDL_GetWindowSizeInPixels(window.get(), &width_in_pixels, &height_in_pixels))
      throw Exception("Failed to get SDL window pixel size: {}", SDL_GetError());

    return {
      .device = device_,
      .format = surface_capabilities.formats[0],
      .width = static_cast<uint32_t>(width_in_pixels),
      .height = static_cast<uint32_t>(height_in_pixels),
      // Essentially enables VSync and is supported on all platforms
      .presentMode = wgpu::PresentMode::Fifo,
    };
  });

  surface_.Configure(&surface_config_);

  // Queue is created at the same time as the device so it must exist at this call
  queue_ = device_.GetQueue();
}

Renderer::~Renderer() { surface_.Unconfigure(); }

const wgpu::Device& Renderer::device() { return device_; }

const wgpu::Surface& Renderer::surface() { return surface_; }

const wgpu::SurfaceConfiguration& Renderer::surface_config() { return surface_config_; }

const wgpu::Queue& Renderer::queue() { return queue_; }

}
