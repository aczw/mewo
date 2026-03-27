#include "context.hpp"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <string>

namespace mewo::gui {

Context::Context(
  const std::filesystem::path& assets_dir,
  const Window& window,
  const gfx::Gfx& gfx
) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.IniFilename = nullptr;

  auto inter_path = assets_dir / "fonts/inter_4.1/Inter-Regular.ttf";
  auto geist_mono_path = assets_dir / "fonts/geist_mono_1.7/GeistMono-Regular.ttf";

  fonts_ = {
    .inter = io.Fonts->AddFontFromFileTTF(inter_path.string().c_str()),
    .geist_mono = io.Fonts->AddFontFromFileTTF(geist_mono_path.string().c_str()),
  };

  io.ConfigDpiScaleFonts = true;
  io.ConfigDpiScaleViewports = true;

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.FontSizeBase = 15.f;

  ImGui_ImplWGPU_InitInfo wgpu_init_info;
  wgpu_init_info.Device = gfx.device().Get();
  wgpu_init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(gfx.surface_config().format);
  ImGui_ImplWGPU_Init(&wgpu_init_info);

  ImGui_ImplSDL3_InitForOther(window.get());

  // Can be set once upfront because there's only one viewport
  viewport_ = ImGui::GetMainViewport();
}

void Context::record(const gfx::FrameContext& frame_ctx) const {
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

}  // namespace mewo::gui
