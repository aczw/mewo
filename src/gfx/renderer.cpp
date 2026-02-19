#include "renderer.hpp"

#include "exception.hpp"
#include "query.hpp"
#include "utility.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <array>
#include <functional>
#include <limits>
#include <optional>
#include <string_view>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#endif

namespace mewo::gfx {

namespace {

constexpr auto WGPU_WAIT_TIMEOUT_MAX = std::numeric_limits<uint64_t>::max();

std::string_view get_surface_texture_status(wgpu::SurfaceGetCurrentTextureStatus status)
{
  switch (status) {
    // clang-format off
  case wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal: return "SuccessOptimal";
  case wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal: return "SuccessSuboptimal";
  case wgpu::SurfaceGetCurrentTextureStatus::Timeout: return "Timeout";
  case wgpu::SurfaceGetCurrentTextureStatus::Outdated: return "Viewportdated";
  case wgpu::SurfaceGetCurrentTextureStatus::Lost: return "Lost";
  case wgpu::SurfaceGetCurrentTextureStatus::Error: return "Error";
    // clang-format on

  default:
    utility::enum_unreachable("wgpu::SurfaceGetCurrentTextureStatus", status);
  }
}

}

Renderer::Renderer(const sdl::Window& window)
{
  auto timed_wait_any = wgpu::InstanceFeatureName::TimedWaitAny;
  wgpu::InstanceDescriptor instance_desc = {
    .requiredFeatureCount = 1,
    .requiredFeatures = &timed_wait_any,
  };

  instance_ = wgpu::CreateInstance(&instance_desc);

  if (!instance_)
    throw Exception("WebGPU instance creation failed");

  wgpu::Adapter adapter;
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

  wgpu::DeviceDescriptor device_desc = { {
      .label = "device",
      .defaultQueue = { .label = "default-queue" },
  } };

  // Dawn-specific functionality to enable/disable certain runtime features
  if constexpr (query::is_debug()) {
    static constexpr std::array DAWN_ENABLED_TOGGLES = { "enable_immediate_error_handling" };

    static const wgpu::DawnTogglesDescriptor DAWN_TOGGLES_DESC = { {
        .enabledToggleCount = DAWN_ENABLED_TOGGLES.size(),
        .enabledToggles = DAWN_ENABLED_TOGGLES.data(),
    } };

    device_desc.nextInChain = &DAWN_TOGGLES_DESC;
  }

  device_desc.SetDeviceLostCallback(
      wgpu::CallbackMode::AllowSpontaneous,
      [](const wgpu::Device&, wgpu::DeviceLostReason type, wgpu::StringView message,
          std::optional<Error>* device_lost_error) {
        auto reason = static_cast<WGPUDeviceLostReason>(type);

        *device_lost_error = {
          .error_type = ImGui_ImplWGPU_GetDeviceLostReasonName(reason),
          .message = std::string(message),
        };
      },
      &device_lost_error_);

  device_desc.SetUncapturedErrorCallback(
      [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message,
          std::optional<Error>* uncaptured_error) {
        auto error_type = static_cast<WGPUErrorType>(type);

        *uncaptured_error = {
          .error_type = ImGui_ImplWGPU_GetErrorTypeName(error_type),
          .message = std::string(message),
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
    surface_.SetLabel("surface");
  }

  surface_config_ = std::invoke([this, &window, &adapter] -> wgpu::SurfaceConfiguration {
    wgpu::SurfaceCapabilities surface_capabilities;

    if (!surface_.GetCapabilities(adapter, &surface_capabilities))
      throw Exception("Failed to get WebGPU surface capabilities");

    auto [width, height] = window.size_in_pixels();

    return {
      .device = device_,
      // There is always at least 1 format if `wgpu::Surface::GetCapabilities` was successful
      .format = surface_capabilities.formats[0],
      .width = width,
      .height = height,
      // Essentially enables VSync and is supported on all platforms
      .presentMode = wgpu::PresentMode::Fifo,
    };
  });

  surface_.Configure(&surface_config_);

  // Queue is created at the same time as the device so it must exist at this call
  queue_ = device_.GetQueue();
}

Renderer::~Renderer() { surface_.Unconfigure(); }

const wgpu::Device& Renderer::device() const { return device_; }

const wgpu::Surface& Renderer::surface() const { return surface_; }

const wgpu::SurfaceConfiguration& Renderer::surface_config() const { return surface_config_; }

const wgpu::Queue& Renderer::queue() const { return queue_; }

FrameContext Renderer::prepare_new_frame() const
{
  if (device_lost_error_.has_value()) {
    const Error& error = device_lost_error_.value();
    throw Exception(
        "WebGPU device lost. Reason: {}. Message (below):\n{}", error.error_type, error.message);
  }

  if (uncaptured_error_.has_value()) {
    const Error& error = uncaptured_error_.value();
    throw Exception(
        "Uncaptured WebGPU error. Type: {}. Message (below):\n{}", error.error_type, error.message);
  }

  wgpu::SurfaceTexture surface_texture;
  surface_.GetCurrentTexture(&surface_texture);

  if (auto status = surface_texture.status;
      status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal
      && status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
    throw Exception("WebGPU surface texture status: {}", get_surface_texture_status(status));
  }

  static const wgpu::TextureViewDescriptor SURFACE_VIEW_DESC = {
    .label = "surface-view",
    .format = surface_config_.format,
    .dimension = wgpu::TextureViewDimension::e2D,
    .aspect = wgpu::TextureAspect::All,
  };

  static const wgpu::CommandEncoderDescriptor COMMAND_ENCODER_DESC = { .label = "command-encoder" };

  return {
    .surface_view = surface_texture.texture.CreateView(&SURFACE_VIEW_DESC),
    .encoder = device_.CreateCommandEncoder(&COMMAND_ENCODER_DESC),
  };
}

void Renderer::resize(uint32_t new_width, uint32_t new_height)
{
  surface_config_.width = new_width;
  surface_config_.height = new_height;

  surface_.Configure(&surface_config_);
}

}
