#include <mewo/mewo.h>

#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>

#include <stdbool.h>

typedef struct {
  mewo_runtime_t* runtime;
} state_t;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("[error] failed to initialize SDL");
    return SDL_APP_FAILURE;
  }

  mewo_string_view_t version = mewo_get_version();
  SDL_Log("[info] Mewo version: %.*s", (int)version.length, version.data);

  state_t* state = SDL_malloc(sizeof(*state));
  *appstate = state;

  if (!state)
    return SDL_APP_FAILURE;

  mewo_result_t res = mewo_runtime_create(&state->runtime);

  if (res < 0) {
    SDL_Log("[error] failed to create Mewo runtime");
    return SDL_APP_FAILURE;
  }

  mewo_runtime_request_device(state->runtime);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  state_t* state = (state_t*)appstate;
  mewo_runtime_t* runtime = state->runtime;

  mewo_runtime_tick(runtime);

  mewo_event_t event;
  while (mewo_runtime_poll_event(runtime, &event)) {
    switch (event.type) {
      case MEWO_EVENT_DEVICE_READY: {
        SDL_Log("[info] device is ready");
        break;
      }

      case MEWO_EVENT_DEVICE_FAILED: {
        mewo_string_view_t* message = &event.variant.device_failed.message;
        SDL_Log("[error] failed to get device: %.*s", (int)message->length, message->data);
        return SDL_APP_FAILURE;
      }

      default: break;
    }
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  (void)appstate;

  if (event->type == SDL_EVENT_QUIT)
    return SDL_APP_SUCCESS;

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  (void)result;

  if (!appstate)
    return;

  state_t* state = (state_t*)appstate;

  mewo_runtime_destroy(state->runtime);
  SDL_free(state);
}
