#pragma once

#include <SDL3/SDL.h>

#include <memory>

namespace mewo::sdl {

/// Mainly serves as a RAII wrapper over SDL and `SDL_Window`. I don't think it makes sense
/// for a window to be copied or moved. Always pass it around as a reference.
class Window {
  public:
  /// Assumes that `SDL_Init` has already been called.
  Window();

  SDL_Window* get() const;

  private:
  struct SDLWindowDeleter {
    void operator()(SDL_Window* window);
  };

  std::unique_ptr<SDL_Window, SDLWindowDeleter> handle;
};

}
