#include "SDL3/SDL_error.h"
#include "SDL3/SDL_video.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <limits>
#include <print>
#include <utility>

namespace {

constexpr auto TIMED_WAIT_ANY = wgpu::InstanceFeatureName::TimedWaitAny;
constexpr uint64_t WAIT_TIMEOUT_MAX = std::numeric_limits<uint64_t>::max();

constexpr auto SDL_INIT_FLAGS = SDL_INIT_VIDEO;

constexpr int SCREEN_W = 1280;
constexpr int SCREEN_H = 720;

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

  int width_in_pixels = -1;
  int height_in_pixels = -1;
  if (!SDL_GetWindowSizeInPixels(window, &width_in_pixels, &height_in_pixels)) {
    std::println(stderr, "Failed to get SDL window size in pixels: {}", SDL_GetError());
    return EXIT_FAILURE;
  }

  wgpu::SurfaceConfiguration surface_configuration = {
    .device = device,
    .format = surface_capabilities.formats[0],
    .usage = wgpu::TextureUsage::RenderAttachment,
    .width = static_cast<uint32_t>(width_in_pixels),
    .height = static_cast<uint32_t>(height_in_pixels),
    .alphaMode = wgpu::CompositeAlphaMode::Auto,
    .presentMode = wgpu::PresentMode::Fifo,
  };

  surface.Configure(&surface_configuration);

  wgpu::Queue queue = device.GetQueue();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForOther(window);

  ImGui_ImplWGPU_InitInfo imgui_wgpu_init_info;
  imgui_wgpu_init_info.Device = device.Get();
  imgui_wgpu_init_info.NumFramesInFlight = 3;
  imgui_wgpu_init_info.RenderTargetFormat
      = static_cast<WGPUTextureFormat>(surface_configuration.format);
  imgui_wgpu_init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
  ImGui_ImplWGPU_Init(&imgui_wgpu_init_info);

  io.Fonts->AddFontDefault();

  bool will_quit = false;
  SDL_Event event = {};

  while (!will_quit) {
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      if (event.type == SDL_EVENT_QUIT) {
        will_quit = true;
      } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED
          && event.window.windowID == SDL_GetWindowID(window)) {
        will_quit = true;
      }
    }

    wgpu::SurfaceTexture surface_texture;
    surface.GetCurrentTexture(&surface_texture);

    if (auto status = static_cast<WGPUSurfaceGetCurrentTextureStatus>(surface_texture.status);
        ImGui_ImplWGPU_IsSurfaceStatusError(status)) {
      std::println(
          stderr, "Unrecoverable surface texture status: {:#x}", std::to_underlying(status));
      std::terminate();
    }

    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();

    wgpu::TextureViewDescriptor texture_view_desc = {
      .format = surface_configuration.format,
      .dimension = wgpu::TextureViewDimension::e2D,
      .aspect = wgpu::TextureAspect::All,
    };

    wgpu::TextureView texture_view = surface_texture.texture.CreateView(&texture_view_desc);

    wgpu::RenderPassColorAttachment color_attachment = {
      .view = texture_view,
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = wgpu::Color { .r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0 },
    };

    wgpu::CommandEncoderDescriptor encoder_desc;
    wgpu::CommandEncoder command_encoder = device.CreateCommandEncoder(&encoder_desc);

    wgpu::RenderPassDescriptor render_pass_desc = {
      .colorAttachmentCount = 1,
      .colorAttachments = &color_attachment,
    };

    wgpu::RenderPassEncoder render_pass_encoder
        = command_encoder.BeginRenderPass(&render_pass_desc);
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), render_pass_encoder.Get());
    render_pass_encoder.End();

    wgpu::CommandBufferDescriptor cmd_buf_desc;
    wgpu::CommandBuffer cmd_buf = command_encoder.Finish(&cmd_buf_desc);

    queue.Submit(1, &cmd_buf);

    surface.Present();
    device.Tick();
  }

  ImGui_ImplWGPU_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  surface.Unconfigure();

  SDL_DestroyWindow(window);
  SDL_QuitSubSystem(SDL_INIT_FLAGS);
  SDL_Quit();

  return EXIT_SUCCESS;
}
