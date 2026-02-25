#pragma once

#include "assets.hpp"
#include "editor.hpp"
#include "gfx/renderer.hpp"
#include "gui/context.hpp"
#include "gui/layout.hpp"
#include "sdl/context.hpp"
#include "sdl/window.hpp"
#include "viewport.hpp"

namespace mewo {

class Mewo {
  public:
  Mewo();

  void run();

  private:
  Assets assets_;
  State state_;

  sdl::Context sdl_ctx_;
  sdl::Window window_;

  gfx::Renderer renderer_;

  gui::Context gui_ctx_;
  gui::Layout layout_;

  Editor editor_;
  Viewport viewport_;
};

}
