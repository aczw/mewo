#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <print>

namespace {

constexpr auto TIMED_WAIT_ANY = wgpu::InstanceFeatureName::TimedWaitAny;
constexpr uint64_t WAIT_TIMEOUT_MAX = std::numeric_limits<uint64_t>::max();

constexpr auto SDL_INIT_FLAGS = SDL_INIT_VIDEO;

constexpr int SCREEN_W = 640;
constexpr int SCREEN_H = 480;

}

int main(int, char*[])
{
  if (!SDL_Init(SDL_INIT_FLAGS)) {
    std::println(stderr, "Failed to initialize SDL: {}", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_WindowFlags window_flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
  SDL_Window* window = SDL_CreateWindow("Mewo 0.0.1-alpha", SCREEN_W, SCREEN_H, window_flags);

  if (!window) {
    std::println(stderr, "Failed to create SDL window: {}", SDL_GetError());
    return EXIT_FAILURE;
  }

  wgpu::InstanceDescriptor instance_desc = {
    .requiredFeatureCount = 1,
    .requiredFeatures = &TIMED_WAIT_ANY,
  };

  wgpu::Instance instance = wgpu::CreateInstance(&instance_desc);

  if (!instance) {
    std::println(stderr, "Instance creation failed");
    return EXIT_FAILURE;
  }

  wgpu::RequestAdapterOptions adapter_opts = {};
  wgpu::Adapter adapter = {};

  auto request_adapter_callback = [&](wgpu::RequestAdapterStatus status,
                                      wgpu::Adapter acquired_adapter, wgpu::StringView message) {
    if (status != wgpu::RequestAdapterStatus::Success) {
      std::println(stderr, "Failed to request an adapter: {}", message.data);
      return;
    }

    adapter = std::move(acquired_adapter);
  };

  wgpu::WaitStatus adapter_wait_status
      = instance.WaitAny(instance.RequestAdapter(&adapter_opts, wgpu::CallbackMode::WaitAnyOnly,
                             request_adapter_callback),
          WAIT_TIMEOUT_MAX);

  if (adapter == nullptr || adapter_wait_status != wgpu::WaitStatus::Success) {
    std::println(stderr, "RequestAdapter failed");
    return EXIT_FAILURE;
  }

  ImGui_ImplWGPU_DebugPrintAdapterInfo(adapter.Get());

  wgpu::DeviceDescriptor device_desc = {};
  device_desc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
      [](const wgpu::Device&, wgpu::DeviceLostReason type, wgpu::StringView message) {
        std::println(stderr, "{} error: {}",
            ImGui_ImplWGPU_GetDeviceLostReasonName(static_cast<WGPUDeviceLostReason>(type)),
            message.data);
      });
  device_desc.SetUncapturedErrorCallback(
      [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
        std::println(stderr, "{} error: {}",
            ImGui_ImplWGPU_GetErrorTypeName(static_cast<WGPUErrorType>(type)), message.data);
      });

  wgpu::Device device = {};

  auto request_device_callback = [&](wgpu::RequestDeviceStatus status, wgpu::Device acquired_device,
                                     wgpu::StringView message) {
    if (status != wgpu::RequestDeviceStatus::Success) {
      std::println(stderr, "Failed to request a device: {}", message.data);
      return;
    }

    device = std::move(acquired_device);
  };

  wgpu::WaitStatus device_wait_status = instance.WaitAny(
      adapter.RequestDevice(&device_desc, wgpu::CallbackMode::WaitAnyOnly, request_device_callback),
      WAIT_TIMEOUT_MAX);

  if (!device || device_wait_status != wgpu::WaitStatus::Success) {
    std::println("RequestDevice failed");
    return EXIT_FAILURE;
  }

  SDL_PropertiesID properties_id = SDL_GetWindowProperties(window);

  if (properties_id == 0) {
    std::println("Failed to get SDL window properties: {}", SDL_GetError());
    return EXIT_FAILURE;
  }

  ImGui_ImplWGPU_CreateSurfaceInfo create_surface_info = {
    .Instance = instance.Get(),
#if defined(SDL_PLATFORM_MACOS)
    .System = "cocoa",
    .RawWindow = static_cast<void*>(
        SDL_GetPointerProperty(properties_id, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr)),
#else
#error "TODO: implement for Windows"
#endif
  };

  wgpu::Surface surface = ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_surface_info);

  if (!surface) {
    std::println(stderr, "Failed to get surface");
    return EXIT_FAILURE;
  }

  wgpu::SurfaceCapabilities surface_capabilities = {};
  if (!surface.GetCapabilities(adapter, &surface_capabilities)) {
    std::println(stderr, "Failed to get surface capabilities");
    return EXIT_FAILURE;
  }

  wgpu::SurfaceConfiguration surface_configuration = {
    .device = device,
    .format = surface_capabilities.formats[0],
    .usage = wgpu::TextureUsage::RenderAttachment,
    .width = SCREEN_W,
    .height = SCREEN_H,
    .alphaMode = wgpu::CompositeAlphaMode::Auto,
    .presentMode = wgpu::PresentMode::Fifo,
  };

  surface.Configure(&surface_configuration);

  wgpu::Queue queue = device.GetQueue();

  bool will_quit = false;
  SDL_Event event = {};

  while (!will_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        will_quit = true;
      }
    }
  }

  SDL_DestroyWindow(window);
  SDL_QuitSubSystem(SDL_INIT_FLAGS);
  SDL_Quit();

  return EXIT_SUCCESS;
}
