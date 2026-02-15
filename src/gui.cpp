#include "gui.hpp"

#include "imgui.h"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <imgui_internal.h>
#include <webgpu/webgpu_cpp.h>

#include <string_view>

namespace mewo {

static constexpr std::string_view MAIN_DOCKSPACE_STR_ID = "main-dockspace";
static constexpr std::string_view EDITOR_WINDOW_NAME = "Editor";
static constexpr std::string_view OUTPUT_WINDOW_NAME = "Output";

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

  // Can be set once upfront because there's only one viewport
  viewport_ = ImGui::GetMainViewport();
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

  ImGuiID dockspace_id = ImGui::GetID(MAIN_DOCKSPACE_STR_ID.data());

  // One time setup, but can only be done after a new frame is initiated
  if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
    set_up_layout(dockspace_id);

  // Submit dockspace layout
  ImGui::DockSpaceOverViewport(dockspace_id, viewport_, ImGuiDockNodeFlags_PassthruCentralNode);

  {
    ImGui::Begin(EDITOR_WINDOW_NAME.data());
    ImGui::Text("I am the editor");
    ImGui::End();
  }

  {
    ImGui::Begin(OUTPUT_WINDOW_NAME.data());
    ImGui::Text("I am the output");
    ImGui::End();
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

void Gui::set_up_layout(ImGuiID dockspace_id) const
{
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, viewport_->Size);

  ImGuiID dock_left_id = {};
  ImGuiID dock_right_id = {};
  ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.5f, &dock_left_id, &dock_right_id);

  ImGui::DockBuilderDockWindow(EDITOR_WINDOW_NAME.data(), dock_left_id);
  ImGui::DockBuilderDockWindow(OUTPUT_WINDOW_NAME.data(), dock_right_id);

  ImGui::DockBuilderFinish(dockspace_id);
}

}
