#include <mewo/mewo.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return EXIT_FAILURE;
  }

  printf("Mewo version: %s", mewo_get_version().data);

  mewo_runtime_t* runtime = NULL;
  mewo_result_t res = mewo_runtime_create(&runtime);

  if (res < 0)
    return res;

  mewo_runtime_destroy(runtime);

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();

  return EXIT_SUCCESS;
}
