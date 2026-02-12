#pragma once

namespace mewo::sdl {

/// Initializes and maintains SDL.
class Context {
  public:
  Context();
  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
};

}
