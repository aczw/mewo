#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <utility>

namespace mewo::sdl {

/// RAII wrapper for `SDL_Window`. Note that copy operations are implicitly deleted
/// because of the `std::unique_ptr` member.
class Window {
  public:
  Window();

  SDL_Window* get() const;

  std::pair<uint32_t, uint32_t> size_in_pixels() const;

  private:
  struct SDLWindowDeleter {
    void operator()(SDL_Window* window);
  };

  std::string title_;
  std::unique_ptr<SDL_Window, SDLWindowDeleter> handle_;
};

}
