#include "window.hpp"

#include "SDL3/SDL_video.h"
#include "exception.hpp"

#include <format>
#include <memory>
#include <string>
#include <string_view>

namespace mewo {

static constexpr auto SDL_INIT_SUBSYSTEMS = SDL_INIT_VIDEO;

Window::Window() {
  if (!SDL_Init(SDL_INIT_SUBSYSTEMS))
    throw Exception("Failed to initialize SDL: {}", SDL_GetError());

  SDL_Window* raw_handle = SDL_CreateWindow(
    title_from_project_name("Untitled project").c_str(),
    1280,
    720,
    SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE
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

}  // namespace mewo
