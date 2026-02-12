#pragma once

#include "gfx/renderer.hpp"
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
};

}
