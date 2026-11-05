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

#define COMMON_FILE_NAME "common.wgsl"
#define FOLDER_SCAN_INTERVAL_MS 250

typedef struct {
  char* path;
  SDL_Time mod_time;
} watched_file_t;

typedef struct {
  char* name;
  watched_file_t file;
  mewo_pass_id_t pass_id;
} pass_file_t;

typedef struct {
  SDL_Window* window;
  SDL_MetalView metal_view;

  mewo_runtime_t* runtime;
  mewo_surface_t* surface;
  mewo_project_t* project;
  mewo_session_t* session;

  const char* folder;

  pass_file_t* pass_files;
  size_t pass_file_count;

  watched_file_t common;
  bool has_common;

  uint64_t last_folder_scan_ms;
  uint64_t last_pass_status_revision;
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

static int SDLCALL compare_strings(const void* a, const void* b) {
  return SDL_strcmp(*(const char* const*)a, *(const char* const*)b);
}

static pass_file_t* find_pass_file_by_name(pass_file_t* files, size_t count, const char* name) {
  for (size_t idx = 0; idx < count; ++idx) {
    // Entries moved out during a sync have their name cleared
    if (files[idx].name && SDL_strcmp(files[idx].name, name) == 0)
      return &files[idx];
  }

  return NULL;
}

static const char* pass_display_name(const state_t* state, mewo_pass_id_t pass_id) {
  for (size_t idx = 0; idx < state->pass_file_count; ++idx) {
    if (state->pass_files[idx].pass_id == pass_id)
      return state->pass_files[idx].name;
  }

  return "(no file)";
}

static void free_pass_file(pass_file_t* file) {
  SDL_free(file->name);
  SDL_free(file->file.path);
  file->name = NULL;
  file->file.path = NULL;
}

static bool add_pass_file(state_t* state, const char* name, pass_file_t* out) {
  pass_file_t added = {0};
  added.name = SDL_strdup(name);

  if (SDL_asprintf(&added.file.path, "%s/%s", state->folder, name) < 0)
    added.file.path = NULL;

  mewo_string_view_t source = {0};

  if (!added.name || !added.file.path || !watched_file_poll(&added.file, &source)) {
    free_pass_file(&added);
    return false;
  }

  mewo_string_view_t pass_name = {.data = added.name, .length = SDL_strlen(added.name)};
  mewo_result_t result = mewo_project_add_pass(state->project, pass_name, source, &added.pass_id);
  SDL_free((void*)source.data);

  if (result < 0) {
    SDL_Log("[error] failed to add a pass for %s", name);
    free_pass_file(&added);
    return false;
  }

  SDL_Log("[info] added pass (ID %u) from %s", added.pass_id, name);
  *out = added;
  return true;
}

static void remove_pass_file(state_t* state, pass_file_t* file) {
  if (mewo_project_remove_pass(state->project, file->pass_id) == MEWO_SUCCESS)
    SDL_Log("[info] removed pass (ID %u), %s is gone", file->pass_id, file->name);
  else
    SDL_Log("[error] failed to remove pass (ID %u)", file->pass_id);

  free_pass_file(file);
}

static bool sync_pass_files(state_t* state) {
  int name_count = 0;
  char** names = SDL_GlobDirectory(state->folder, "*.wgsl", 0, &name_count);

  if (!names)
    return true;

  SDL_qsort(names, (size_t)name_count, sizeof(*names), compare_strings);

  pass_file_t* new_files = NULL;
  size_t new_count = 0;

  if (name_count > 0) {
    new_files = SDL_calloc((size_t)name_count, sizeof(*new_files));

    if (!new_files) {
      SDL_free(names);
      return false;
    }
  }

  for (int name_idx = 0; name_idx < name_count; ++name_idx) {
    const char* name = names[name_idx];

    if (SDL_strcmp(name, COMMON_FILE_NAME) == 0)
      continue;

    pass_file_t* existing = find_pass_file_by_name(state->pass_files, state->pass_file_count, name);

    if (existing) {
      new_files[new_count++] = *existing;
      existing->name = NULL;
      existing->file.path = NULL;
    } else if (add_pass_file(state, name, &new_files[new_count])) {
      new_count += 1;
    }
  }

  SDL_free(names);

  // Entries that weren't moved over belong to files that are gone
  for (size_t idx = 0; idx < state->pass_file_count; ++idx) {
    if (state->pass_files[idx].name)
      remove_pass_file(state, &state->pass_files[idx]);
  }

  SDL_free(state->pass_files);
  state->pass_files = new_files;
  state->pass_file_count = new_count;

  // Put each pass at its file's index. Placing them front to back keeps the earlier ones in place,
  // and moving a pass to its current index is a no-op.
  for (size_t idx = 0; idx < new_count; ++idx) {
    if (mewo_project_move_pass(state->project, new_files[idx].pass_id, (uint32_t)idx) < 0)
      SDL_Log("[error] failed to move pass (ID %u)", new_files[idx].pass_id);
  }

  return true;
}

static bool reload_changed_sources(state_t* state) {
  for (size_t idx = 0; idx < state->pass_file_count; ++idx) {
    pass_file_t* pass_file = &state->pass_files[idx];
    mewo_string_view_t source = {0};

    if (!watched_file_poll(&pass_file->file, &source))
      continue;

    mewo_result_t result = mewo_project_set_pass_source(state->project, pass_file->pass_id, source);
    SDL_free((void*)source.data);

    if (result < 0) {
      SDL_Log("[error] failed to set pass (ID %u) source", pass_file->pass_id);
      return false;
    }

    SDL_Log("[info] reloaded %s", pass_file->name);
  }

  mewo_string_view_t common_source = {0};

  if (watched_file_poll(&state->common, &common_source)) {
    mewo_result_t result = mewo_project_set_common_source(state->project, common_source);
    SDL_free((void*)common_source.data);

    if (result < 0) {
      SDL_Log("[error] failed to set common source");
      return false;
    }

    state->has_common = true;
    SDL_Log("[info] reloaded %s", COMMON_FILE_NAME);
  } else if (state->has_common && get_file_mod_time(state->common.path) < 0) {
    // `common.wgsl` was deleted, so passes stop using it
    mewo_string_view_t empty = {.data = "", .length = 0};

    if (mewo_project_set_common_source(state->project, empty) < 0)
      return false;

    state->has_common = false;
    state->common.mod_time = 0;
    SDL_Log("[info] %s is gone, cleared the common source", COMMON_FILE_NAME);
  }

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

  if (argc > 2) {
    SDL_Log("usage: %s [<shader folder>]", argv[0]);
    return SDL_APP_FAILURE;
  }

  if (argc == 2) {
    SDL_PathInfo info = {0};

    if (!SDL_GetPathInfo(argv[1], &info) || info.type != SDL_PATHTYPE_DIRECTORY) {
      SDL_Log("[error] %s is not a folder", argv[1]);
      SDL_Log("usage: %s [<shader folder>]", argv[0]);
      return SDL_APP_FAILURE;
    }

    state->folder = argv[1];

    if (SDL_asprintf(&state->common.path, "%s/%s", state->folder, COMMON_FILE_NAME) < 0)
      return SDL_APP_FAILURE;

    SDL_Log("[info] watching %s", state->folder);
  } else {
    SDL_Log("[info] no shader folder given");
  }

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

  // Load the folder before the session exists, so its first frame already has every pass
  if (state->folder) {
    mewo_pass_id_t initial_pass_id = mewo_project_get_pass_id(state->project, 0);

    if (!sync_pass_files(state) || !reload_changed_sources(state))
      return SDL_APP_FAILURE;

    if (mewo_project_remove_pass(state->project, initial_pass_id) < 0)
      SDL_Log(
        "[error] failed to remove the initial pass (does %s have any passes?)", state->folder
      );

    state->last_folder_scan_ms = SDL_GetTicks();
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

        state->last_pass_status_revision = 0;

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

  if (state->folder) {
    uint64_t now_ms = SDL_GetTicks();

    if (now_ms - state->last_folder_scan_ms >= FOLDER_SCAN_INTERVAL_MS) {
      if (!sync_pass_files(state))
        return SDL_APP_FAILURE;

      state->last_folder_scan_ms = now_ms;
    }

    if (!reload_changed_sources(state))
      return SDL_APP_FAILURE;
  }

  if (!state->session)
    return SDL_APP_CONTINUE;

  uint64_t pass_status_revision = mewo_session_get_pass_status_revision(state->session);

  if (pass_status_revision != state->last_pass_status_revision) {
    uint32_t pass_count = mewo_project_get_pass_count(state->project);

    for (uint32_t pass_idx = 0; pass_idx < pass_count; ++pass_idx) {
      mewo_pass_id_t pass_id = mewo_project_get_pass_id(state->project, pass_idx);
      mewo_pass_compile_status_t pass_compile_status = MEWO_PASS_COMPILE_STATUS_PENDING;

      // The session only learns about new passes on its next render, so it may not know this one
      if (mewo_session_get_pass_compile_status(state->session, pass_id, &pass_compile_status) < 0)
        continue;

      switch (pass_compile_status) {
        case MEWO_PASS_COMPILE_STATUS_ERROR:
        case MEWO_PASS_COMPILE_STATUS_SUCCESS: {
          const char* pass_name = pass_display_name(state, pass_id);
          uint32_t diagnostic_count = 0;
          mewo_session_get_pass_diagnostic_count(state->session, pass_id, &diagnostic_count);

          for (uint32_t diag_idx = 0; diag_idx < diagnostic_count; ++diag_idx) {
            mewo_diagnostic_t diagnostic = {0};
            mewo_session_get_pass_diagnostic(state->session, pass_id, diag_idx, &diagnostic);

            if (diagnostic.source == MEWO_DIAGNOSTIC_SOURCE_NONE) {
              SDL_Log(
                "[diag][%s] %s (pass %u): %.*s",
                severity_name(diagnostic.severity),
                pass_name,
                pass_id,
                (int)diagnostic.message.length,
                diagnostic.message.data
              );
            } else {
              // Common lines are relative to `common.wgsl`, so name that file instead of the pass
              const char* file_name =
                diagnostic.source == MEWO_DIAGNOSTIC_SOURCE_COMMON ? COMMON_FILE_NAME : pass_name;

              SDL_Log(
                "[diag][%s][%s] %s (pass %u) %" SDL_PRIu64 ":%" SDL_PRIu64 ": %.*s",
                severity_name(diagnostic.severity),
                source_name(diagnostic.source),
                file_name,
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

    state->last_pass_status_revision = pass_status_revision;
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

  for (size_t idx = 0; idx < state->pass_file_count; ++idx)
    free_pass_file(&state->pass_files[idx]);

  SDL_free(state->pass_files);
  SDL_free(state->common.path);

  if (state->metal_view)
    SDL_Metal_DestroyView(state->metal_view);
  SDL_DestroyWindow(state->window);
  SDL_free(state);
}
