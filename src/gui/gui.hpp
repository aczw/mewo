#pragma once

#include "event/event_queue.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/gfx.hpp"
#include "gui/editor/editor.hpp"
#include "gui/theme.hpp"
#include "viewport.hpp"
#include "window.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <filesystem>

namespace mewo {

class Gui {
 public:
  static constexpr float SPLIT_LEFT_RATIO = 0.5f;

  Gui(const std::filesystem::path& assets_dir, const Window& window, const gfx::Gfx& gfx);

  ~Gui() {
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }

  Theme theme() const { return theme_; }

  void begin_frame() const {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
  }

  /// Builds the GUI and records additional data into respective classes. Called every frame.
  void build_layout(
    EventQueue& event_queue,
    const gfx::FrameContext& frame_ctx,
    Editor& editor,
    Viewport& viewport
  );

  void update(const gfx::FrameContext& frame_ctx) const;

 private:
  struct Fonts {
    ImFont* inter = nullptr;
    ImFont* geist_mono = nullptr;
  };

  void build_main_menu_bar(EventQueue& event_queue, Editor& editor);
  void build_editor(Editor& editor) const;
  void build_diagnostics(Editor& editor) const;
  void build_viewport(
    EventQueue& event_queue,
    const gfx::FrameContext& frame_ctx,
    Viewport& viewport,
    Editor& editor
  );

  /// Sets up the overall docking layout. Only needs to be called once. Can only
  /// be called after a new frame is initiated, so it's not possible in the constructor.
  void set_up_initial_layout(ImGuiID dockspace_id) const;

  /// Needs to be cached every frame. Will be checked to see if the viewport texture
  /// needs to be resized. Only relevant when the viewport mode is `AspectRatio`.
  uint32_t prev_viewport_window_width_ = 0;
  ImGuiViewport* imgui_viewport_ = nullptr;

  Fonts fonts_;
  Theme theme_ = Theme::RosePine;
};

}  // namespace mewo
