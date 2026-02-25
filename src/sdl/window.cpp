#include "window.hpp"

#include "exception.hpp"
#include "query.hpp"

#include <string>

namespace mewo::sdl {

static std::string get_window_title()
{
  return std::string(query::is_debug() ? "[DEBUG] " : "") + "Mewo v" + query::version_full().data();
}

Window::Window()
    : handle_(SDL_CreateWindow(get_window_title().c_str(), 1280, 720,
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

void Window::SDLWindowDeleter::operator()(SDL_Window* window) { SDL_DestroyWindow(window); }

}
