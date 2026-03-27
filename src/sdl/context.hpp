#pragma once

#include "exception.hpp"

#include <SDL3/SDL_init.h>

namespace mewo::sdl {

/// Initializes and maintains SDL.
class Context {
 public:
  static constexpr auto SDL_SUBSYSTEMS = SDL_INIT_VIDEO;

  Context() {
    if (!SDL_Init(SDL_SUBSYSTEMS))
      throw Exception("Failed to initialize SDL: {}", SDL_GetError());
  }

  ~Context() {
    SDL_QuitSubSystem(SDL_SUBSYSTEMS);
    SDL_Quit();
  }

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
};

}  // namespace mewo::sdl
