#include "mewo.hpp"

#include "exception.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo {

Mewo::Mewo()
    : sdl_ctx_()
    , window_()
    , renderer_(window_)
{
}

void Mewo::run()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontDefault();

  ImGui::StyleColorsDark();

  ImGui_ImplWGPU_InitInfo imgui_wgpu_init_info;
  imgui_wgpu_init_info.Device = renderer_.device().Get();
  imgui_wgpu_init_info.RenderTargetFormat
      = static_cast<WGPUTextureFormat>(renderer_.surface_config().format);
  ImGui_ImplWGPU_Init(&imgui_wgpu_init_info);
  ImGui_ImplSDL3_InitForOther(window_.get());

  bool will_quit = false;
  SDL_Event event = {};

  while (!will_quit) {
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      if (event.type == SDL_EVENT_QUIT
          || (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED
              && event.window.windowID == SDL_GetWindowID(window_.get()))) {
        will_quit = true;
      }
    }

    wgpu::SurfaceTexture surface_texture;
    renderer_.surface().GetCurrentTexture(&surface_texture);

    if (surface_texture.status == wgpu::SurfaceGetCurrentTextureStatus::Error)
      throw Exception(
          "WebGPU surface texture error: {:#x}", std::to_underlying(surface_texture.status));

    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    ImGui::Render();

    wgpu::CommandEncoderDescriptor encoder_desc;
    wgpu::CommandEncoder command_encoder = renderer_.device().CreateCommandEncoder(&encoder_desc);

    wgpu::TextureViewDescriptor texture_view_desc = {
      .format = renderer_.surface_config().format,
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
    renderer_.queue().Submit(1, &cmd_buf);

    renderer_.surface().Present();

    renderer_.device().Tick();
  }

  ImGui_ImplWGPU_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

}
