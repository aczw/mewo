#pragma once

#include "exception.hpp"
#include "project.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <memory>
#include <string>
#include <utility>

namespace mewo {

class Window {
 public:
  Window();
  ~Window();

  [[nodiscard]] SDL_Window* get() const { return handle_.get(); }

  std::pair<uint32_t, uint32_t> size_in_pixels() const;

  void update_project_in_title(const Project& project);
  void update_dirty_status_in_title(bool is_dirty);

 private:
  struct Destructor {
    void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
  };

  std::string create_title() const;

  void update_title() const {
    if (!SDL_SetWindowTitle(handle_.get(), create_title().c_str()))
      throw Exception("Failed to set window title: {}", SDL_GetError());
  }

  std::unique_ptr<SDL_Window, Destructor> handle_;

  std::string cached_project_name_;
  bool cached_is_dirty_ = false;
};

}  // namespace mewo
