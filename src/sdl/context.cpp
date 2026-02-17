#include "context.hpp"

#include "exception.hpp"

#include <SDL3/SDL.h>

namespace mewo::sdl {

static constexpr auto SDL_SUBSYSTEMS = SDL_INIT_VIDEO;

Context::Context()
{
  if (!SDL_Init(SDL_SUBSYSTEMS))
    throw Exception("Failed to initialize SDL: {}", SDL_GetError());
}

Context::~Context()
{
  SDL_QuitSubSystem(SDL_SUBSYSTEMS);
  SDL_Quit();
}

}
