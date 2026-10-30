#include <mewo/mewo.h>

#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>

typedef struct {
  SDL_Window* window;
  SDL_MetalView metal_view;

  mewo_runtime_t* runtime;
  mewo_surface_t* surface;
  mewo_project_t* project;
} state_t;

static bool fill_surface_desc(state_t* state, mewo_surface_descriptor_t* desc) {
  int w = 0;
  int h = 0;
  SDL_GetWindowSizeInPixels(state->window, &w, &h);
  desc->initial_width = (uint32_t)w;
  desc->initial_height = (uint32_t)h;

#if defined(SDL_PLATFORM_MACOS)
  state->metal_view = SDL_Metal_CreateView(state->window);
  desc->type = MEWO_SURFACE_SOURCE_METAL_LAYER;
  desc->variant.metal_layer.layer = SDL_Metal_GetLayer(state->metal_view);
#elif defined(SDL_PLATFORM_WINDOWS)
  SDL_PropertiesID props = SDL_GetWindowProperties(state->window);
  desc->type = MEWO_SURFACE_SOURCE_WIN32;
  desc->variant.win32.hwnd =
    SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
  desc->variant.win32.hinstance =
    SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, NULL);
#else
  SDL_Log("[error] platform not supported or invalid");
  return false;
#endif
  return true;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("[error] failed to initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  mewo_string_view_t version = mewo_get_version();
  SDL_Log("[info] Mewo version: %.*s", (int)version.length, version.data);

  state_t* state = SDL_calloc(1, sizeof(*state));
  *appstate = state;

  if (!state)
    return SDL_APP_FAILURE;

  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(SDL_PLATFORM_MACOS)
  window_flags |= SDL_WINDOW_METAL;
#endif
  state->window = SDL_CreateWindow("Mewo", 1280, 720, window_flags);

  if (!state->window) {
    SDL_Log("[error] failed to create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (mewo_runtime_create(&state->runtime) < 0)
    return SDL_APP_FAILURE;

  mewo_surface_descriptor_t surface_desc = {0};
  if (!fill_surface_desc(state, &surface_desc))
    return SDL_APP_FAILURE;
  if (mewo_surface_create(state->runtime, &surface_desc, &state->surface) < 0)
    return SDL_APP_FAILURE;

  if (mewo_project_create(&state->project) < 0)
    return SDL_APP_FAILURE;

  mewo_runtime_request_device(state->runtime, state->surface);

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
  state_t* state = (state_t*)appstate;

  switch (event->type) {
    case SDL_EVENT_QUIT: return SDL_APP_SUCCESS;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
      SDL_Log("[info] window resized to %ix%i", event->window.data1, event->window.data2);
      mewo_surface_resize(
        state->surface, (uint32_t)event->window.data1, (uint32_t)event->window.data2
      );
      break;
    }

    default: break;
  }

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  (void)result;

  if (!appstate)
    return;

  state_t* state = (state_t*)appstate;

  mewo_project_destroy(state->project);
  mewo_surface_destroy(state->surface);
  mewo_runtime_destroy(state->runtime);
  if (state->metal_view)
    SDL_Metal_DestroyView(state->metal_view);
  SDL_DestroyWindow(state->window);
  SDL_free(state);
}
