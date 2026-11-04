#include <mewo/mewo.h>

#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>

typedef struct {
  const char* path;
  SDL_Time mod_time;
} watched_file_t;

typedef struct {
  SDL_Window* window;
  SDL_MetalView metal_view;

  mewo_runtime_t* runtime;
  mewo_surface_t* surface;
  mewo_project_t* project;
  mewo_session_t* session;

  bool watch_mode;
  watched_file_t pass_1;
  mewo_pass_id_t pass_1_id;
  watched_file_t common;
  uint64_t last_status_revision;
} state_t;

static bool fill_surface_desc(state_t* state, mewo_surface_descriptor_t* desc) {
  int window_width = 0;
  int window_height = 0;
  SDL_GetWindowSizeInPixels(state->window, &window_width, &window_height);
  desc->initial_width = (uint32_t)window_width;
  desc->initial_height = (uint32_t)window_height;

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

static SDL_Time get_file_mod_time(const char* file) {
  SDL_PathInfo info = {0};

  if (!SDL_GetPathInfo(file, &info))
    return -1;

  return info.modify_time;
}

static bool watched_file_poll(watched_file_t* file, mewo_string_view_t* out) {
  SDL_Time mod_time = get_file_mod_time(file->path);

  if (mod_time <= 0 || mod_time == file->mod_time)
    return false;

  size_t size = 0;
  void* data = SDL_LoadFile(file->path, &size);

  if (!data)
    return false;

  file->mod_time = mod_time;
  *out = (mewo_string_view_t){.data = data, .length = size};

  return true;
}

static const char* severity_name(mewo_diagnostic_severity_t severity) {
  switch (severity) {
    case MEWO_DIAGNOSTIC_SEVERITY_ERROR: return "error";
    case MEWO_DIAGNOSTIC_SEVERITY_WARNING: return "warning";
    case MEWO_DIAGNOSTIC_SEVERITY_INFO: return "info";
    default: return "unknown";
  }
}

static const char* source_name(mewo_diagnostic_source_t source) {
  switch (source) {
    case MEWO_DIAGNOSTIC_SOURCE_PASS: return "pass";
    case MEWO_DIAGNOSTIC_SOURCE_COMMON: return "common";
    case MEWO_DIAGNOSTIC_SOURCE_GENERATED: return "generated";
    default: return "unknown";
  }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
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

  SDL_Log("[info] created new project");

  state->pass_1_id = mewo_project_get_pass_id(state->project, 0);

  if (argc == 1) {
    state->watch_mode = false;
    SDL_Log("[info] watch mode = false");
  } else if (argc > 3) {
    SDL_Log("usage: %s <shader.wgsl> [<common.wgsl>]", argv[0]);
    return SDL_APP_FAILURE;
  } else {
    state->watch_mode = true;
    SDL_Log("[info] watch mode = true");

    state->pass_1.path = argv[1];
    state->pass_1.mod_time = 0;

    SDL_Log("[info] observing %s...", state->pass_1.path);

    if (argc == 3) {
      state->common.path = argv[2];
      state->common.mod_time = 0;

      SDL_Log("[info] common source: %s", state->common.path);
    } else {
      state->common.path = NULL;
    }
  }

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

        int window_width = 0;
        int window_height = 0;
        SDL_GetWindowSizeInPixels(state->window, &window_width, &window_height);

        mewo_session_descriptor_t session_desc = {
          .project = state->project,
          .render_width = (uint32_t)window_width,
          .render_height = (uint32_t)window_height,
        };

        if (mewo_session_create(state->runtime, &session_desc, &state->session) < 0)
          return SDL_APP_FAILURE;

        state->last_status_revision = 0;

        SDL_Log("[info] created new session");

        break;
      }

      case MEWO_EVENT_DEVICE_FAILED: {
        mewo_string_view_t* message = &event.variant.device_failed.message;
        SDL_Log("[error] failed to get device: %.*s", (int)message->length, message->data);
        return SDL_APP_FAILURE;
      }

      case MEWO_EVENT_DEVICE_LOST: {
        mewo_string_view_t* message = &event.variant.device_lost.message;
        SDL_Log("[error] device lost: %.*s", (int)message->length, message->data);
        return SDL_APP_FAILURE;
      }

      case MEWO_EVENT_GPU_ERROR: {
        mewo_string_view_t* message = &event.variant.gpu_error.message;
        SDL_Log("[error] GPU error: %.*s", (int)message->length, message->data);
        break;
      }

      default: break;
    }
  }

  if (state->watch_mode) {
    mewo_string_view_t pass_1_source = {0};
    if (watched_file_poll(&state->pass_1, &pass_1_source)) {
      if (mewo_project_set_pass_source(state->project, state->pass_1_id, pass_1_source) < 0) {
        SDL_Log("[error] failed to set pass (ID %u) source", state->pass_1_id);
        SDL_free((void*)pass_1_source.data);
        return SDL_APP_FAILURE;
      }

      SDL_Log("[info] reloaded %s", state->pass_1.path);
      SDL_free((void*)pass_1_source.data);
    }

    if (state->common.path) {
      mewo_string_view_t common_source = {0};
      if (watched_file_poll(&state->common, &common_source)) {
        if (mewo_project_set_common_source(state->project, common_source) < 0) {
          SDL_Log("[error] failed to set common source");
          SDL_free((void*)common_source.data);
          return SDL_APP_FAILURE;
        }

        SDL_Log("[info] reloaded %s", state->common.path);
        SDL_free((void*)common_source.data);
      }
    }
  }

  if (!state->session)
    return SDL_APP_CONTINUE;

  uint64_t status_revision = mewo_session_get_status_revision(state->session);

  if (status_revision != state->last_status_revision) {
    uint32_t pass_count = mewo_project_get_pass_count(state->project);

    for (uint32_t pass_idx = 0; pass_idx < pass_count; ++pass_idx) {
      mewo_pass_id_t pass_id = mewo_project_get_pass_id(state->project, pass_idx);
      mewo_pass_compile_status_t pass_compile_status = MEWO_PASS_COMPILE_STATUS_PENDING;
      mewo_session_get_pass_compile_status(state->session, pass_id, &pass_compile_status);

      switch (pass_compile_status) {
        case MEWO_PASS_COMPILE_STATUS_ERROR:
        case MEWO_PASS_COMPILE_STATUS_SUCCESS: {
          uint32_t diagnostic_count = 0;
          mewo_session_get_pass_diagnostic_count(state->session, pass_id, &diagnostic_count);

          for (uint32_t diag_idx = 0; diag_idx < diagnostic_count; ++diag_idx) {
            mewo_diagnostic_t diagnostic = {0};
            mewo_session_get_pass_diagnostic(state->session, pass_id, diag_idx, &diagnostic);

            if (diagnostic.source == MEWO_DIAGNOSTIC_SOURCE_NONE) {
              SDL_Log(
                "[diag][%s] Pass %u: %.*s",
                severity_name(diagnostic.severity),
                pass_id,
                (int)diagnostic.message.length,
                diagnostic.message.data
              );
            } else {
              SDL_Log(
                "[diag][%s][%s] Pass %u (%" SDL_PRIu64 ":%" SDL_PRIu64 "): %.*s",
                severity_name(diagnostic.severity),
                source_name(diagnostic.source),
                pass_id,
                diagnostic.line,
                diagnostic.column,
                (int)diagnostic.message.length,
                diagnostic.message.data
              );
            }
          }

          break;
        }

        default: break;
      }
    }

    state->last_status_revision = status_revision;
  }

  if (mewo_session_render(state->session, state->surface, SDL_GetTicksNS()) < 0)
    return SDL_APP_FAILURE;

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  state_t* state = (state_t*)appstate;

  switch (event->type) {
    case SDL_EVENT_QUIT: return SDL_APP_SUCCESS;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
      uint32_t new_width = (uint32_t)event->window.data1;
      uint32_t new_height = (uint32_t)event->window.data2;

      SDL_Log("[info] window resized to %i×%i", event->window.data1, event->window.data2);

      mewo_surface_resize(state->surface, new_width, new_height);

      if (state->session && new_width != 0 && new_height != 0) {
        if (mewo_session_set_render_size(state->session, new_width, new_height) < 0)
          return SDL_APP_FAILURE;
      }
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

  mewo_session_destroy(state->session);
  mewo_project_destroy(state->project);
  mewo_surface_destroy(state->surface);
  mewo_runtime_destroy(state->runtime);

  if (state->metal_view)
    SDL_Metal_DestroyView(state->metal_view);
  SDL_DestroyWindow(state->window);
  SDL_free(state);
}
