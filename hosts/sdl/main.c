#include <mewo/mewo.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  if (!SDL_Init(SDL_INIT_VIDEO))
    return EXIT_FAILURE;

  printf("[info] Mewo version: %s\n", mewo_get_version().data);

  int exit_code = EXIT_SUCCESS;
  mewo_runtime_t* runtime = NULL;
  mewo_result_t res = mewo_runtime_create(&runtime);

  mewo_runtime_request_device(runtime);
  mewo_event_t event;

  while (true) {
    mewo_runtime_tick(runtime);
    bool device_is_ready = false;

    while (mewo_runtime_poll_event(runtime, &event)) {
      switch (event.type) {
        case MEWO_EVENT_DEVICE_READY: {
          printf("[info] device is ready\n");
          device_is_ready = true;
          break;
        }

        case MEWO_EVENT_DEVICE_FAILED: {
          printf("[error] failed to get device: %s\n", event.variant.device_failed.message.data);
          exit_code = EXIT_FAILURE;
          goto cleanup;
        }

        default: break;
      }
    }

    if (device_is_ready)
      break;
  }

  if (res < 0) {
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

cleanup:
  mewo_runtime_destroy(runtime);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();

  return exit_code;
}
