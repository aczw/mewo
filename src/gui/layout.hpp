#pragma once

#include "editor.hpp"
#include "gui/context.hpp"
#include "out.hpp"

#include <imgui.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo {

/// Breaks circular dependency.
class Mewo;

namespace gui {

class Layout {
  public:
  /// Builds the layout. Called every frame.
  void build(Mewo& ctx, const Context& gui_ctx, const wgpu::Device& device, Editor& editor,
      Out& out) const;

  private:
  /// Sets up the overall docking layout. Only needs to be called once. Can only
  /// be called after a new frame is initiated, so it's not possible in the constructor.
  void set_up_initial_layout(const Context& gui_ctx, ImGuiID dockspace_id) const;
};

}

}
