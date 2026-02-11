#include "window.hpp"

#include "exception.hpp"

#include <print>

namespace mewo::sdl {

namespace {

constexpr int SCREEN_W = 1280;
constexpr int SCREEN_H = 720;

constexpr auto SDL_WINDOW_FLAGS = SDL_WINDOW_HIGH_PIXEL_DENSITY;

}

Window::Window()
    : handle(SDL_CreateWindow("Mewo 0.0.1-alpha", SCREEN_W, SCREEN_H, SDL_WINDOW_FLAGS))
{
  std::println("hello!");
  if (!handle) {
    throw Exception("Failed to create SDL window: {}", SDL_GetError());
  }
}

SDL_Window* Window::get() const { return handle.get(); }

void Window::SDLWindowDeleter::operator()(SDL_Window* window) { SDL_DestroyWindow(window); }

}
