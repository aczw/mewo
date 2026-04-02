#include "gfx.hpp"

#include "exception.hpp"
#include "os.hpp"
#include "query.hpp"
#include "util/enum_unreachable.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <array>
#include <optional>
#include <print>
#include <string_view>

namespace mewo::gfx {

namespace {

std::string_view get_surface_texture_status(wgpu::SurfaceGetCurrentTextureStatus status) {
  switch (status) {
    case wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal: return "SuccessOptimal";
    case wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal: return "SuccessSuboptimal";
    case wgpu::SurfaceGetCurrentTextureStatus::Timeout: return "Timeout";
    case wgpu::SurfaceGetCurrentTextureStatus::Outdated: return "Viewportdated";
    case wgpu::SurfaceGetCurrentTextureStatus::Lost: return "Lost";
    case wgpu::SurfaceGetCurrentTextureStatus::Error: return "Error";

    default: util::enum_unreachable("wgpu::SurfaceGetCurrentTextureStatus", status);
  }
}

}  // namespace

Gfx::Gfx(const Window& window) {
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

  wgpu::WaitStatus adapter_status = instance_.WaitAny(
    instance_.RequestAdapter(
      &adapter_opts,
      wgpu::CallbackMode::WaitAnyOnly,
      [&adapter](
        wgpu::RequestAdapterStatus status, wgpu::Adapter acquired_adapter, wgpu::StringView message
      ) {
        // Throwing here is safe because we wait on callback execution in the current thread
        if (status != wgpu::RequestAdapterStatus::Success)
          throw Exception("Failed to request WebGPU adapter: {}", message.data);

        adapter = std::move(acquired_adapter);
      }
    ),
    WAIT_TIMEOUT_MAX
  );

  if (!adapter || adapter_status != wgpu::WaitStatus::Success)
    throw Exception("Waiting on wgpu::Instance::RequestAdapter failed");

  if constexpr (query::is_debug())
    ImGui_ImplWGPU_DebugPrintAdapterInfo(adapter.Get());

  wgpu::DeviceDescriptor device_desc = {{
    .label = "device",
    .defaultQueue = {.label = "default-queue"},
  }};

  // Dawn-specific functionality to enable/disable certain runtime features
  if constexpr (query::is_debug()) {
    static constexpr std::array DAWN_ENABLED_TOGGLES = {"enable_immediate_error_handling"};

    static const wgpu::DawnTogglesDescriptor DAWN_TOGGLES_DESC = {{
      .enabledToggleCount = DAWN_ENABLED_TOGGLES.size(),
      .enabledToggles = DAWN_ENABLED_TOGGLES.data(),
    }};

    device_desc.nextInChain = &DAWN_TOGGLES_DESC;
  }

  device_desc.SetDeviceLostCallback(
    wgpu::CallbackMode::AllowSpontaneous,
    [](
      const wgpu::Device&,
      wgpu::DeviceLostReason type,
      wgpu::StringView message,
      std::optional<Error>* device_lost_error
    ) {
      auto reason = static_cast<WGPUDeviceLostReason>(type);

      *device_lost_error = {
        .type_name = ImGui_ImplWGPU_GetDeviceLostReasonName(reason),
        .message = std::string(message),
      };
    },
    &device_lost_error_
  );

  device_desc.SetUncapturedErrorCallback(
    [](
      const wgpu::Device&,
      wgpu::ErrorType type,
      wgpu::StringView message,
      std::optional<Error>* uncaptured_error
    ) {
      auto error_type = static_cast<WGPUErrorType>(type);

      *uncaptured_error = {
        .type_name = ImGui_ImplWGPU_GetErrorTypeName(error_type),
        .message = std::string(message),
      };
    },
    &uncaptured_error_
  );

  wgpu::WaitStatus device_status = instance_.WaitAny(
    adapter.RequestDevice(
      &device_desc,
      wgpu::CallbackMode::WaitAnyOnly,
      [this](
        wgpu::RequestDeviceStatus status, wgpu::Device acquired_device, wgpu::StringView message
      ) {
        // Throwing here is safe because we wait on callback execution in the current thread
        if (status != wgpu::RequestDeviceStatus::Success)
          throw Exception("Failed to request WebGPU device: {}", message.data);

        device_ = std::move(acquired_device);
      }
    ),
    WAIT_TIMEOUT_MAX
  );

  if (!device_ || device_status != wgpu::WaitStatus::Success)
    throw Exception("Waiting on wgpu::Adapter::RequestDevice failed");

  auto create_surface_info = os::retrieve_surface_info(instance_, window);

  if (
    WGPUSurface raw_surface = ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_surface_info);
    !raw_surface
  ) {
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

FrameContext Gfx::begin_frame() {
  if (device_lost_error_.has_value()) {
    const Error& error = device_lost_error_.value();
    throw Exception(
      "WebGPU device lost. Reason: {}. Message (below):\n{}", error.type_name, error.message
    );
  }

  if (uncaptured_error_.has_value()) {
    const Error& error = uncaptured_error_.value();
    std::println(
      "Uncaptured WebGPU error. Type: {}. Message (below):\n{}", error.type_name, error.message
    );
    uncaptured_error_.reset();
  }

  wgpu::SurfaceTexture surface_texture;
  surface_.GetCurrentTexture(&surface_texture);

  if (
    auto status = surface_texture.status;
    status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
    status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal
  ) {
    throw Exception("WebGPU surface texture status: {}", get_surface_texture_status(status));
  } else if (status == wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
    std::println("warning: surface texture is suboptimal");
  }

  static const wgpu::TextureViewDescriptor SURFACE_VIEW_DESC = {
    .label = "surface-view",
    .format = surface_config_.format,
    .dimension = wgpu::TextureViewDimension::e2D,
    .aspect = wgpu::TextureAspect::All,
  };

  static constexpr wgpu::CommandEncoderDescriptor CMD_ENCODER_DESC = {.label = "command-encoder"};

  return {
    .surface_view = surface_texture.texture.CreateView(&SURFACE_VIEW_DESC),
    .encoder = device_.CreateCommandEncoder(&CMD_ENCODER_DESC),
  };
}

}  // namespace mewo::gfx
