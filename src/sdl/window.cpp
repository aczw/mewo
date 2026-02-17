#include "window.hpp"

#include "exception.hpp"

namespace mewo::sdl {

Window::Window()
    : handle_(SDL_CreateWindow("Mewo 0.0.1-alpha", 1280, 720, SDL_WINDOW_HIGH_PIXEL_DENSITY))
{
  if (!handle_)
    throw Exception("Failed to create SDL window: {}", SDL_GetError());
}

SDL_Window* Window::get() const { return handle_.get(); }

void Window::SDLWindowDeleter::operator()(SDL_Window* window) { SDL_DestroyWindow(window); }

}
