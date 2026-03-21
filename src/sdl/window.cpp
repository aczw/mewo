#include "window.hpp"

#include "SDL3/SDL_video.h"
#include "exception.hpp"
#include "query.hpp"

#include <format>
#include <string>
#include <string_view>

namespace mewo::sdl {

static std::string title_from_project_name(std::string_view name)
{
  return std::format(
      "{}{} — Mewo {}", query::is_debug() ? "[DEBUG] " : "", name, query::version_full());
}

Window::Window()
    : handle_(SDL_CreateWindow(title_from_project_name("Untitled project").c_str(), 1280, 720,
          SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE))
{
  if (!handle_)
    throw Exception("Failed to create SDL window: {}", SDL_GetError());
}

SDL_Window* Window::get() const { return handle_.get(); }

std::pair<uint32_t, uint32_t> Window::size_in_pixels() const
{
  int width_in_pixels = 0;
  int height_in_pixels = 0;

  if (!SDL_GetWindowSizeInPixels(handle_.get(), &width_in_pixels, &height_in_pixels))
    throw Exception("Failed to get SDL window pixel size: {}", SDL_GetError());

  return { static_cast<uint32_t>(width_in_pixels), static_cast<uint32_t>(height_in_pixels) };
}

void Window::set_project_in_title(const Project& project)
{
  SDL_SetWindowTitle(handle_.get(), title_from_project_name(project.name()).c_str());
}

void Window::SDLWindowDeleter::operator()(SDL_Window* window) { SDL_DestroyWindow(window); }

}
