#include "gui.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo {

Gui::Gui(const sdl::Window& window, const gfx::Renderer& renderer)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontDefault();

  ImGui::StyleColorsDark();

  ImGui_ImplWGPU_InitInfo wgpu_init_info;
  wgpu_init_info.Device = renderer.device().Get();
  wgpu_init_info.RenderTargetFormat
      = static_cast<WGPUTextureFormat>(renderer.surface_config().format);
  ImGui_ImplWGPU_Init(&wgpu_init_info);

  ImGui_ImplSDL3_InitForOther(window.get());
}

Gui::~Gui()
{
  ImGui_ImplWGPU_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

void Gui::record(const gfx::FrameContext& frame_ctx) const
{
  ImGui_ImplWGPU_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  {
    // This is where I would actually create the UI but currently I'm just
    // showing a demo window
    ImGui::ShowDemoWindow();
  }

  ImGui::Render();

  auto& [surface_view, encoder] = frame_ctx;

  wgpu::RenderPassColorAttachment color_attachment = {
    .view = surface_view,
    .loadOp = wgpu::LoadOp::Load,
    .storeOp = wgpu::StoreOp::Store,
  };

  wgpu::RenderPassDescriptor render_pass_desc = {
    .label = "imgui-render-pass",
    .colorAttachmentCount = 1,
    .colorAttachments = &color_attachment,
  };

  wgpu::RenderPassEncoder render_pass = encoder.BeginRenderPass(&render_pass_desc);
  ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), render_pass.Get());
  render_pass.End();
}

}
