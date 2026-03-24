#pragma once

#include "project.hpp"
#include "query.hpp"

#include <SDL3/SDL_video.h>

#include <format>
#include <memory>
#include <utility>

namespace mewo::sdl {

/// RAII wrapper for `SDL_Window`. Note that copy operations are implicitly deleted
/// because of the `std::unique_ptr` member.
class Window {
  public:
  Window();

  SDL_Window* get() const { return handle_.get(); }

  std::pair<uint32_t, uint32_t> size_in_pixels() const;

  void update_project_in_title(const Project& project)
  {
    SDL_SetWindowTitle(handle_.get(), title_from_project_name(project.name()).c_str());
  }

  private:
  struct SDLWindowDeleter {
    void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
  };

  std::string title_from_project_name(std::string_view name)
  {
    return std::format(
        "{}{} — Mewo {}", query::is_debug() ? "[DEBUG] " : "", name, query::version_full());
  }

  std::unique_ptr<SDL_Window, SDLWindowDeleter> handle_;
};

}
