#pragma once

#include "editor.hpp"
#include "gui/context.hpp"
#include "state.hpp"
#include "viewport.hpp"

#include <imgui.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo::gui {

class Layout {
  public:
  /// Builds the GUI and records additional data into respective classes. Called every frame.
  void build(State& state, const Context& gui_ctx, const wgpu::Device& device, Editor& editor,
      Viewport& viewport);

  private:
  /// Sets up the overall docking layout. Only needs to be called once. Can only
  /// be called after a new frame is initiated, so it's not possible in the constructor.
  void set_up_initial_layout(const Context& gui_ctx, ImGuiID dockspace_id) const;

  /// Needs to be cached every frame. Will be checked to see if the viewport texture
  /// needs to be resized. Only relevant when the viewport mode is `AspectRatio`.
  uint32_t prev_viewport_window_width_ = 0;
};

}
