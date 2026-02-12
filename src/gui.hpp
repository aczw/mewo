#pragma once

#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"
#include "sdl/window.hpp"

#include <webgpu/webgpu_cpp.h>

namespace mewo {

class Gui {
  public:
  Gui(const sdl::Window& window, const gfx::Renderer& renderer);
  ~Gui();

  Gui(const Gui&) = delete;
  Gui& operator=(const Gui&) = delete;

  void record(const gfx::FrameContext& frame_ctx) const;
};

}
