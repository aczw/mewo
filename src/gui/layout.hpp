#pragma once

#include "gui/context.hpp"
#include "out.hpp"

#include <imgui.h>

namespace mewo::gui {

class Layout {
  public:
  /// Builds the layout. Called every frame.
  void build(const Context& gui_ctx, Out& out) const;

  private:
  /// Sets up the overall docking layout. Only needs to be called once.
  void set_up_initial_layout(const Context& gui_ctx, ImGuiID dockspace_id) const;
};

}
