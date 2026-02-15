#pragma once

#include "gfx/renderer.hpp"
#include "gui/context.hpp"
#include "gui/layout.hpp"
#include "out.hpp"
#include "sdl/context.hpp"
#include "sdl/window.hpp"

namespace mewo {

class Mewo {
  public:
  Mewo();

  void run();

  private:
  sdl::Context sdl_ctx_;
  sdl::Window window_;

  gfx::Renderer renderer_;

  gui::Context gui_ctx_;
  gui::Layout layout_;

  Out out_;
};

}
