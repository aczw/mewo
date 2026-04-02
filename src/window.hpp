#pragma once

#include "exception.hpp"
#include "project.hpp"
#include "query.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <format>
#include <memory>
#include <utility>

namespace mewo {

class Window {
 public:
  Window();
  ~Window();

  [[nodiscard]] SDL_Window* get() const { return handle_.get(); }

  std::pair<uint32_t, uint32_t> size_in_pixels() const;

  void update_project_in_title(const Project& project) const {
    if (!SDL_SetWindowTitle(handle_.get(), title_from_project_name(project.name()).c_str()))
      throw Exception("Failed to set window title: {}", SDL_GetError());
  }

 private:
  struct Destructor {
    void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
  };

  std::string title_from_project_name(std::string_view name) const {
    return std::format(
      "{}{} — Mewo {}", query::is_debug() ? "[DEBUG] " : "", name, query::version_full()
    );
  }

  std::unique_ptr<SDL_Window, Destructor> handle_;
};

}  // namespace mewo
