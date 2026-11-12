#include "wrappers.h"

#include <mewo/mewo.h>

#include <emscripten/em_macros.h>
#include <emscripten/emscripten.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** One buffer for all string results; see `to_c_string()`. */
static char* string_buffer = NULL;
static size_t string_capacity = 0;

static mewo_result_t last_result = MEWO_SUCCESS;
static mewo_event_t current_event = {0};

static const char* to_c_string(mewo_string_view_t view) {
  if (view.data == NULL)
    return "";

  size_t length = view.length == MEWO_STRING_VIEW_SENTINEL ? strlen(view.data) : view.length;

  if (length + 1 > string_capacity) {
    char* grown_string = realloc(string_buffer, length + 1);

    if (grown_string == NULL)
      return "";

    string_buffer = grown_string;
    string_capacity = length + 1;
  }

  memcpy(string_buffer, view.data, length);
  string_buffer[length] = '\0';

  return string_buffer;
}

static mewo_string_view_t to_string_view(const char* string) {
  return (mewo_string_view_t){
    .data = string,
    .length = MEWO_STRING_VIEW_SENTINEL,
  };
}

EMSCRIPTEN_KEEPALIVE
int mewo_web_last_result(void) { return (int)last_result; }

EMSCRIPTEN_KEEPALIVE
const char* mewo_web_get_version(void) { return to_c_string(mewo_get_version()); }

EMSCRIPTEN_KEEPALIVE
mewo_runtime_t* mewo_web_runtime_create(void) {
  mewo_runtime_t* runtime = NULL;
  last_result = mewo_runtime_create(&runtime);
  return runtime;
}

EMSCRIPTEN_KEEPALIVE
void mewo_web_runtime_destroy(mewo_runtime_t* runtime) { mewo_runtime_destroy(runtime); }

EMSCRIPTEN_KEEPALIVE void mewo_web_runtime_tick(mewo_runtime_t* runtime) {
  mewo_runtime_tick(runtime);
}

EMSCRIPTEN_KEEPALIVE
void mewo_web_runtime_request_device(mewo_runtime_t* runtime, mewo_surface_t* surface) {
  mewo_runtime_request_device(runtime, surface);
}

EMSCRIPTEN_KEEPALIVE
int mewo_web_runtime_poll_event(mewo_runtime_t* runtime) {
  if (!mewo_runtime_poll_event(runtime, &current_event))
    current_event = (mewo_event_t){.type = MEWO_EVENT_NONE};

  return (int)current_event.type;
}

EMSCRIPTEN_KEEPALIVE
const char* mewo_web_event_message(void) {
  switch (current_event.type) {
    case MEWO_EVENT_DEVICE_FAILED: return to_c_string(current_event.variant.device_failed.message);
    case MEWO_EVENT_DEVICE_LOST: return to_c_string(current_event.variant.device_lost.message);
    case MEWO_EVENT_GPU_ERROR: return to_c_string(current_event.variant.gpu_error.message);
    default: return "";
  }
}

EMSCRIPTEN_KEEPALIVE
mewo_surface_t* mewo_web_surface_create_canvas(
  mewo_runtime_t* runtime,
  const char* selector,
  uint32_t width,
  uint32_t height
) {
  mewo_surface_descriptor_t desc = {0};
  desc.type = MEWO_SURFACE_SOURCE_HTML_CANVAS;
  desc.initial_width = width;
  desc.initial_height = height;
  desc.variant.html_canvas.selector = to_string_view(selector);

  mewo_surface_t* surface = NULL;
  last_result = mewo_surface_create(runtime, &desc, &surface);
  return surface;
}

EMSCRIPTEN_KEEPALIVE
void mewo_web_surface_destroy(mewo_surface_t* surface) { mewo_surface_destroy(surface); }

EMSCRIPTEN_KEEPALIVE
void mewo_web_surface_resize(mewo_surface_t* surface, uint32_t width, uint32_t height) {
  mewo_surface_resize(surface, width, height);
}

EMSCRIPTEN_KEEPALIVE
mewo_project_t* mewo_web_project_create(
  const char* first_pass_name,
  const char* first_pass_source
) {
  mewo_project_descriptor_t desc = {0};
  desc.first_pass_name = to_string_view(first_pass_name);
  desc.first_pass_source = to_string_view(first_pass_source);

  mewo_project_t* project = NULL;
  last_result = mewo_project_create(&desc, &project);
  return project;
}

EMSCRIPTEN_KEEPALIVE
void mewo_web_project_destroy(mewo_project_t* project) { mewo_project_destroy(project); }

EMSCRIPTEN_KEEPALIVE
bool mewo_web_project_set_pass_source(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  const char* source
) {
  last_result = mewo_project_set_pass_source(project, pass_id, to_string_view(source));
  return last_result == MEWO_SUCCESS;
}

EMSCRIPTEN_KEEPALIVE
bool mewo_web_project_set_pass_name(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  const char* name
) {
  last_result = mewo_project_set_pass_name(project, pass_id, to_string_view(name));
  return last_result == MEWO_SUCCESS;
}

EMSCRIPTEN_KEEPALIVE
bool mewo_web_project_set_common_source(mewo_project_t* project, const char* source) {
  last_result = mewo_project_set_common_source(project, to_string_view(source));
  return last_result == MEWO_SUCCESS;
}

EMSCRIPTEN_KEEPALIVE
mewo_pass_id_t mewo_web_project_add_pass(
  mewo_project_t* project,
  const char* name,
  const char* source
) {
  mewo_pass_id_t pass_id = MEWO_PASS_ID_NONE;
  last_result =
    mewo_project_add_pass(project, to_string_view(name), to_string_view(source), &pass_id);
  return pass_id;
}

EMSCRIPTEN_KEEPALIVE
bool mewo_web_project_remove_pass(mewo_project_t* project, mewo_pass_id_t pass_id) {
  last_result = mewo_project_remove_pass(project, pass_id);
  return last_result == MEWO_SUCCESS;
}

EMSCRIPTEN_KEEPALIVE
bool mewo_web_project_move_pass(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  uint32_t new_index
) {
  last_result = mewo_project_move_pass(project, pass_id, new_index);
  return last_result == MEWO_SUCCESS;
}

EMSCRIPTEN_KEEPALIVE
uint32_t mewo_web_project_get_pass_count(mewo_project_t* project) {
  return mewo_project_get_pass_count(project);
}

EMSCRIPTEN_KEEPALIVE
mewo_pass_id_t mewo_web_project_get_pass_id(mewo_project_t* project, uint32_t index) {
  return mewo_project_get_pass_id(project, index);
}

EMSCRIPTEN_KEEPALIVE
const char* mewo_web_project_get_pass_name(mewo_project_t* project, mewo_pass_id_t pass_id) {
  mewo_string_view_t name = {0};
  last_result = mewo_project_get_pass_name(project, pass_id, &name);

  if (last_result != MEWO_SUCCESS)
    return NULL;

  return to_c_string(name);
}

EMSCRIPTEN_KEEPALIVE
const char* mewo_web_project_get_pass_source(mewo_project_t* project, mewo_pass_id_t pass_id) {
  mewo_string_view_t source = {0};
  last_result = mewo_project_get_pass_source(project, pass_id, &source);

  if (last_result != MEWO_SUCCESS)
    return NULL;

  return to_c_string(source);
}

EMSCRIPTEN_KEEPALIVE
const char* mewo_web_project_get_common_source(mewo_project_t* project) {
  return to_c_string(mewo_project_get_common_source(project));
}

EMSCRIPTEN_KEEPALIVE
mewo_session_t* mewo_web_session_create(
  mewo_runtime_t* runtime,
  mewo_project_t* project,
  uint32_t width,
  uint32_t height
) {
  mewo_session_descriptor_t desc = {0};
  desc.project = project;
  desc.render_width = width;
  desc.render_height = height;

  mewo_session_t* session = NULL;
  last_result = mewo_session_create(runtime, &desc, &session);
  return session;
}

EMSCRIPTEN_KEEPALIVE
void mewo_web_session_destroy(mewo_session_t* session) { mewo_session_destroy(session); }

EMSCRIPTEN_KEEPALIVE
bool mewo_web_session_set_render_size(mewo_session_t* session, uint32_t width, uint32_t height) {
  last_result = mewo_session_set_render_size(session, width, height);
  return last_result == MEWO_SUCCESS;
}

EMSCRIPTEN_KEEPALIVE
bool mewo_web_session_render(mewo_session_t* session, mewo_surface_t* surface, double time_ms) {
  last_result = mewo_session_render(session, surface, (uint64_t)(time_ms * 1e6));
  return last_result == MEWO_SUCCESS;
}

EMSCRIPTEN_KEEPALIVE
double mewo_web_session_get_time(mewo_session_t* session) {
  return (double)mewo_session_get_time(session) / 1e6;
}

EMSCRIPTEN_KEEPALIVE
void mewo_web_session_set_paused(mewo_session_t* session, bool paused) {
  mewo_session_set_paused(session, paused);
}

EMSCRIPTEN_KEEPALIVE
bool mewo_web_session_is_paused(mewo_session_t* session) { return mewo_session_is_paused(session); }

EMSCRIPTEN_KEEPALIVE
double mewo_web_session_get_pass_compile_duration(mewo_session_t* session, mewo_pass_id_t pass_id) {
  uint64_t duration_ns = 0;
  last_result = mewo_session_get_pass_compile_duration(session, pass_id, &duration_ns);

  if (last_result != MEWO_SUCCESS)
    return -1;

  return (double)duration_ns / 1e6;
}
