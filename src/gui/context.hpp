#pragma once

#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"
#include "sdl/window.hpp"

#include <imgui.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo::gui {

/// Immediate mode GUI rendering using Dear ImGui.
class Context {
  public:
  Context(const sdl::Window& window, const gfx::Renderer& renderer);
  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  void record(const gfx::FrameContext& frame_ctx) const;

  private:
  void set_up_layout(ImGuiID dockspace_id) const;

  ImGuiViewport* viewport_ = nullptr;
};

}
