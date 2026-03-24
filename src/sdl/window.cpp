#include "window.hpp"

#include "SDL3/SDL_video.h"
#include "exception.hpp"

#include <format>
#include <string>
#include <string_view>

namespace mewo::sdl {

Window::Window()
    : handle_(SDL_CreateWindow(title_from_project_name("Untitled project").c_str(), 1280, 720,
          SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE))
{
  if (!handle_)
    throw Exception("Failed to create SDL window: {}", SDL_GetError());
}

std::pair<uint32_t, uint32_t> Window::size_in_pixels() const
{
  int width_in_pixels = 0;
  int height_in_pixels = 0;

  if (!SDL_GetWindowSizeInPixels(handle_.get(), &width_in_pixels, &height_in_pixels))
    throw Exception("Failed to get SDL window pixel size: {}", SDL_GetError());

  return { static_cast<uint32_t>(width_in_pixels), static_cast<uint32_t>(height_in_pixels) };
}

}
