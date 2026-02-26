#pragma once

#include "assets.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"
#include "sdl/window.hpp"

#include <imgui.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo::gui {

/// Immediate mode GUI rendering using Dear ImGui.
class Context {
  public:
  Context(const Assets& assets, const sdl::Window& window, const gfx::Renderer& renderer);
  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  const ImGuiViewport* viewport() const;

  void prepare_new_frame() const;
  void record(const gfx::FrameContext& frame_ctx) const;

  private:
  ImGuiViewport* viewport_ = nullptr;
};

}
