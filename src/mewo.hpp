#pragma once

#include "sdl/context.hpp"
#include "sdl/window.hpp"

namespace mewo {

class Mewo {
  public:
  void run();

  private:
  sdl::Context sdl_ctx;
  sdl::Window window;
};

}
