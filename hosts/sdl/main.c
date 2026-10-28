#include <mewo/mewo.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>

#include <stdio.h>

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return 1;
  }

  printf("Mewo version: %s", mewo_get_version().data);

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();

  return 0;
}
