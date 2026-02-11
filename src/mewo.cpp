#include "mewo.hpp"

#include "exception.hpp"
#include "query.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#endif

namespace mewo {

namespace {

constexpr uint64_t WGPU_WAIT_TIMEOUT_MAX = std::numeric_limits<uint64_t>::max();

}

void Mewo::run()
{
  auto timed_wait_any = wgpu::InstanceFeatureName::TimedWaitAny;
  wgpu::InstanceDescriptor instance_desc = {
    .requiredFeatureCount = 1,
    .requiredFeatures = &timed_wait_any,
  };

  wgpu::Instance instance = wgpu::CreateInstance(&instance_desc);

  if (!instance)
    throw Exception("WebGPU instance creation failed");

  wgpu::Adapter adapter = {};

  wgpu::RequestAdapterOptions adapter_opts;
  wgpu::WaitStatus adapter_wait_status = instance.WaitAny(
      instance.RequestAdapter(&adapter_opts, wgpu::CallbackMode::WaitAnyOnly,
          [&](wgpu::RequestAdapterStatus status, wgpu::Adapter acquired_adapter,
              wgpu::StringView message) {
            if (status != wgpu::RequestAdapterStatus::Success)
              throw Exception("Failed to request WebGPU adapter: {}", message.data);

            adapter = std::move(acquired_adapter);
          }),
      WGPU_WAIT_TIMEOUT_MAX);

  if (!adapter || adapter_wait_status != wgpu::WaitStatus::Success)
    throw Exception("Call to wgpu::Instance::RequestAdapter failed");

  if constexpr (query::is_debug())
    ImGui_ImplWGPU_DebugPrintAdapterInfo(adapter.Get());

  wgpu::Device device = {};

  // TODO: revisit once I have better error handling methods (I probably shouldn't just throw)
  wgpu::DeviceDescriptor device_desc;
  device_desc.SetDeviceLostCallback(wgpu::CallbackMode::AllowProcessEvents,
      [](const wgpu::Device&, wgpu::DeviceLostReason type, wgpu::StringView message) {
        throw Exception("WebGPU device lost. {} error: {}",
            ImGui_ImplWGPU_GetDeviceLostReasonName(static_cast<WGPUDeviceLostReason>(type)),
            message.data);
      });
  device_desc.SetUncapturedErrorCallback(
      [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
        throw Exception("Uncaptured WebGPU error. {} error: {}",
            ImGui_ImplWGPU_GetErrorTypeName(static_cast<WGPUErrorType>(type)), message.data);
      });

  wgpu::WaitStatus device_wait_status = instance.WaitAny(
      adapter.RequestDevice(&device_desc, wgpu::CallbackMode::WaitAnyOnly,
          [&](wgpu::RequestDeviceStatus status, wgpu::Device acquired_device,
              wgpu::StringView message) {
            if (status != wgpu::RequestDeviceStatus::Success)
              throw Exception("Failed to request WebGPU device: {}", message.data);

            device = std::move(acquired_device);
          }),
      WGPU_WAIT_TIMEOUT_MAX);

  if (!device || device_wait_status != wgpu::WaitStatus::Success)
    throw Exception("Call to wgpu::Adapter::RequestDevice failed");

  SDL_PropertiesID properties_id = SDL_GetWindowProperties(window.get());

  if (properties_id == 0)
    throw Exception("Failed to get SDL window properties: {}", SDL_GetError());

  ImGui_ImplWGPU_CreateSurfaceInfo create_surface_info = {
    .Instance = instance.Get(),
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

  wgpu::Surface surface = ImGui_ImplWGPU_CreateWGPUSurfaceHelper(&create_surface_info);

  if (!surface)
    throw Exception("Failed to create WebGPU surface");

  wgpu::SurfaceCapabilities surface_capabilities;
  if (!surface.GetCapabilities(adapter, &surface_capabilities))
    throw Exception("Failed to get WebGPU surface capabilities");
  if (surface_capabilities.formatCount == 0)
    throw Exception("No available WebGPU surface formats");

  int width_in_pixels = 0;
  int height_in_pixels = 0;
  if (!SDL_GetWindowSizeInPixels(window.get(), &width_in_pixels, &height_in_pixels))
    throw Exception("Failed to get SDL window pixel size: {}", SDL_GetError());

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

  // Queue is created at the same time as the device so it must exist here
  wgpu::Queue queue = device.GetQueue();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontDefault();

  ImGui::StyleColorsDark();

  ImGui_ImplWGPU_InitInfo imgui_wgpu_init_info;
  imgui_wgpu_init_info.Device = device.Get();
  imgui_wgpu_init_info.RenderTargetFormat
      = static_cast<WGPUTextureFormat>(surface_configuration.format);
  ImGui_ImplWGPU_Init(&imgui_wgpu_init_info);
  ImGui_ImplSDL3_InitForOther(window.get());

  bool will_quit = false;
  SDL_Event event = {};

  while (!will_quit) {
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      if (event.type == SDL_EVENT_QUIT
          || (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED
              && event.window.windowID == SDL_GetWindowID(window.get()))) {
        will_quit = true;
      }
    }

    wgpu::SurfaceTexture surface_texture;
    surface.GetCurrentTexture(&surface_texture);

    if (surface_texture.status == wgpu::SurfaceGetCurrentTextureStatus::Error)
      throw Exception(
          "WebGPU surface texture error: {:#x}", std::to_underlying(surface_texture.status));

    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    ImGui::Render();

    wgpu::CommandEncoderDescriptor encoder_desc;
    wgpu::CommandEncoder command_encoder = device.CreateCommandEncoder(&encoder_desc);

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
}

}
