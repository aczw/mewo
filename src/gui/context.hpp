#pragma once

#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"
#include "sdl/window.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <filesystem>

namespace mewo::gui {

/// Immediate mode GUI rendering using Dear ImGui.
class Context {
 public:
  struct Fonts {
    ImFont* inter = nullptr;
    ImFont* geist_mono = nullptr;
  };

  Context(
    const std::filesystem::path& assets_dir,
    const sdl::Window& window,
    const gfx::Renderer& renderer
  );

  ~Context() {
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  const ImGuiViewport* viewport() const { return viewport_; }

  const Fonts& fonts() const { return fonts_; }

  void prepare_new_frame() const {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
  }

  void record(const gfx::FrameContext& frame_ctx) const;

 private:
  ImGuiViewport* viewport_ = nullptr;
  Fonts fonts_;
};

}  // namespace mewo::gui
