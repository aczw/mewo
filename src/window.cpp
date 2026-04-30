#include "window.hpp"

#include "exception.hpp"
#include "query.hpp"

#include <SDL3/SDL_video.h>

#include <format>
#include <memory>
#include <string>

namespace mewo {

static constexpr auto SDL_INIT_SUBSYSTEMS = SDL_INIT_VIDEO;

Window::Window() {
  if (!SDL_Init(SDL_INIT_SUBSYSTEMS))
    throw Exception("Failed to initialize SDL: {}", SDL_GetError());

  cached_project_name_ = "Untitled project";

  SDL_Window* raw_handle = SDL_CreateWindow(
    create_title().c_str(), 1280, 720, SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE
  );

  if (raw_handle == nullptr)
    throw Exception("Failed to create SDL window: {}", SDL_GetError());

  handle_.reset(raw_handle);
}

Window::~Window() {
  SDL_QuitSubSystem(SDL_INIT_SUBSYSTEMS);
  SDL_Quit();
}

std::pair<uint32_t, uint32_t> Window::size_in_pixels() const {
  int width_in_pixels = 0;
  int height_in_pixels = 0;

  if (!SDL_GetWindowSizeInPixels(handle_.get(), &width_in_pixels, &height_in_pixels))
    throw Exception("Failed to get SDL window pixel size: {}", SDL_GetError());

  return {static_cast<uint32_t>(width_in_pixels), static_cast<uint32_t>(height_in_pixels)};
}

void Window::update_project_in_title(const Project& project) {
  if (const auto& project_name = project.name(); project_name == cached_project_name_) {
    return;
  } else {
    cached_project_name_ = project_name;
    cached_is_dirty_ = false;  // A new project implies dirty state is reset
  }

  update_title();
}

void Window::update_dirty_status_in_title(bool is_dirty) {
  if (is_dirty == cached_is_dirty_) {
    return;
  }

  cached_is_dirty_ = is_dirty;
  update_title();
}

std::string Window::create_title() const {
  return std::format(
    "{}{}{} — Mewo {}",
    query::is_debug() ? "[DEBUG] " : "",
    cached_project_name_,
    cached_is_dirty_ ? "*" : "",
    query::version_full()
  );
}

}  // namespace mewo
