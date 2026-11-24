#include <mewo/mewo.h>

#include <emscripten/em_macros.h>
#include <emscripten/emscripten.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * JS-friendly bindings over `mewo.h`. Conventions:
 *
 * - Handles are passed and returned as numbers in JS. `NULL` = 0, unsurprisingly.
 * - Functions always return their value directly.
 *   Functions that can fail store the result, which JS reads with `last_result()`.
 * - Strings in are NUL-terminated UTF-8, allocated by JS and freed after the call.
 * - Strings out are NUL-terminated and stay valid until the next binding call that returns a
 *   string, so JS copies them right away.
 */

/** One buffer for all string results. */
static char* string_buffer = NULL;
static size_t string_capacity = 0;

static mewo_result_t stored_result = MEWO_SUCCESS;
static mewo_event_t current_event = {0};
static mewo_diagnostic_t current_diagnostic = {0};

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

/** The `mewo_result_t` of the most recent failable binding call. */
EMSCRIPTEN_KEEPALIVE
int web_last_result(void) { return (int)stored_result; }

EMSCRIPTEN_KEEPALIVE
const char* web_get_version(void) { return to_c_string(mewo_get_version()); }

EMSCRIPTEN_KEEPALIVE
mewo_runtime_t* runtime_create(void) {
  mewo_runtime_t* runtime = NULL;
  stored_result = mewo_runtime_create(&runtime);
  return runtime;
}

EMSCRIPTEN_KEEPALIVE
void runtime_destroy(mewo_runtime_t* runtime) { mewo_runtime_destroy(runtime); }

EMSCRIPTEN_KEEPALIVE void runtime_tick(mewo_runtime_t* runtime) { mewo_runtime_tick(runtime); }

EMSCRIPTEN_KEEPALIVE
void runtime_request_device(mewo_runtime_t* runtime, mewo_surface_t* surface) {
  mewo_runtime_request_device(runtime, surface);
}

/**
 * Pops one event and keeps it as the current event, readable through the `event_*`
 * getters. Returns its `mewo_event_type_t`, or `MEWO_EVENT_NONE` if the queue is empty.
 */
EMSCRIPTEN_KEEPALIVE
int runtime_poll_event(mewo_runtime_t* runtime) {
  if (!mewo_runtime_poll_event(runtime, &current_event))
    current_event = (mewo_event_t){.type = MEWO_EVENT_NONE};

  return (int)current_event.type;
}

/** The current event's message, or "" if it has none. */
EMSCRIPTEN_KEEPALIVE
const char* event_message(void) {
  switch (current_event.type) {
    case MEWO_EVENT_DEVICE_FAILED: return to_c_string(current_event.variant.device_failed.message);
    case MEWO_EVENT_DEVICE_LOST: return to_c_string(current_event.variant.device_lost.message);
    case MEWO_EVENT_GPU_ERROR: return to_c_string(current_event.variant.gpu_error.message);
    default: return "";
  }
}

/**
 * Initial size in device pixels.
 */
EMSCRIPTEN_KEEPALIVE
mewo_surface_t* surface_create_canvas(
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
  stored_result = mewo_surface_create(runtime, &desc, &surface);
  return surface;
}

EMSCRIPTEN_KEEPALIVE
void surface_destroy(mewo_surface_t* surface) { mewo_surface_destroy(surface); }

/** Resizes the surface to the new size in device pixels. 0 is valid. */
EMSCRIPTEN_KEEPALIVE
void surface_resize(mewo_surface_t* surface, uint32_t width, uint32_t height) {
  mewo_surface_resize(surface, width, height);
}

EMSCRIPTEN_KEEPALIVE
mewo_project_t* project_create(
  const char* name,
  const char* first_pass_name,
  const char* first_pass_source
) {
  mewo_project_descriptor_t desc = {0};
  desc.name = to_string_view(name);
  desc.first_pass_name = to_string_view(first_pass_name);
  desc.first_pass_source = to_string_view(first_pass_source);

  mewo_project_t* project = NULL;
  stored_result = mewo_project_create(&desc, &project);
  return project;
}

EMSCRIPTEN_KEEPALIVE
void project_destroy(mewo_project_t* project) { mewo_project_destroy(project); }

EMSCRIPTEN_KEEPALIVE
const char* project_get_name(mewo_project_t* project) {
  return to_c_string(mewo_project_get_name(project));
}

EMSCRIPTEN_KEEPALIVE
void project_set_name(mewo_project_t* project, const char* name) {
  stored_result = mewo_project_set_name(project, to_string_view(name));
}

/** Wraps `mewo_check_project_name`, which needs no project. */
EMSCRIPTEN_KEEPALIVE
int project_check_name(const char* name) {
  mewo_name_check_t check = MEWO_NAME_CHECK_OK;
  stored_result = mewo_check_project_name(to_string_view(name), &check);
  return (int)check;
}

EMSCRIPTEN_KEEPALIVE
void project_set_pass_source(mewo_project_t* project, mewo_pass_id_t pass_id, const char* source) {
  stored_result = mewo_project_set_pass_source(project, pass_id, to_string_view(source));
}

EMSCRIPTEN_KEEPALIVE
void project_set_pass_name(mewo_project_t* project, mewo_pass_id_t pass_id, const char* name) {
  stored_result = mewo_project_set_pass_name(project, pass_id, to_string_view(name));
}

/** `pass_id` is `MEWO_PASS_ID_NONE` when checking a name for a new pass. */
EMSCRIPTEN_KEEPALIVE
int project_check_pass_name(mewo_project_t* project, mewo_pass_id_t pass_id, const char* name) {
  mewo_name_check_t check = MEWO_NAME_CHECK_OK;
  stored_result = mewo_project_check_pass_name(project, pass_id, to_string_view(name), &check);
  return (int)check;
}

EMSCRIPTEN_KEEPALIVE
void project_set_common_source(mewo_project_t* project, const char* source) {
  stored_result = mewo_project_set_common_source(project, to_string_view(source));
}

EMSCRIPTEN_KEEPALIVE
mewo_pass_id_t project_add_pass(mewo_project_t* project, const char* name, const char* source) {
  mewo_pass_id_t pass_id = MEWO_PASS_ID_NONE;
  stored_result =
    mewo_project_add_pass(project, to_string_view(name), to_string_view(source), &pass_id);
  return pass_id;
}

EMSCRIPTEN_KEEPALIVE
void project_remove_pass(mewo_project_t* project, mewo_pass_id_t pass_id) {
  stored_result = mewo_project_remove_pass(project, pass_id);
}

EMSCRIPTEN_KEEPALIVE
void project_move_pass(mewo_project_t* project, mewo_pass_id_t pass_id, uint32_t new_index) {
  stored_result = mewo_project_move_pass(project, pass_id, new_index);
}

EMSCRIPTEN_KEEPALIVE
uint32_t project_get_pass_count(mewo_project_t* project) {
  return mewo_project_get_pass_count(project);
}

EMSCRIPTEN_KEEPALIVE
mewo_pass_id_t project_get_pass_id(mewo_project_t* project, uint32_t index) {
  return mewo_project_get_pass_id(project, index);
}

EMSCRIPTEN_KEEPALIVE
const char* project_get_pass_name(mewo_project_t* project, mewo_pass_id_t pass_id) {
  mewo_string_view_t name = {0};
  stored_result = mewo_project_get_pass_name(project, pass_id, &name);
  return to_c_string(name);
}

EMSCRIPTEN_KEEPALIVE
const char* project_get_pass_source(mewo_project_t* project, mewo_pass_id_t pass_id) {
  mewo_string_view_t source = {0};
  stored_result = mewo_project_get_pass_source(project, pass_id, &source);
  return to_c_string(source);
}

EMSCRIPTEN_KEEPALIVE
const char* project_get_common_source(mewo_project_t* project) {
  return to_c_string(mewo_project_get_common_source(project));
}

/** Passing 0 for the width and height is invalid. */
EMSCRIPTEN_KEEPALIVE
mewo_session_t* session_create(
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
  stored_result = mewo_session_create(runtime, &desc, &session);
  return session;
}

EMSCRIPTEN_KEEPALIVE
void session_destroy(mewo_session_t* session) { mewo_session_destroy(session); }

/** Passing 0 for the width and height is invalid. */
EMSCRIPTEN_KEEPALIVE
void session_set_render_size(mewo_session_t* session, uint32_t width, uint32_t height) {
  stored_result = mewo_session_set_render_size(session, width, height);
}

EMSCRIPTEN_KEEPALIVE
void session_render(mewo_session_t* session, mewo_surface_t* surface, double time_ms) {
  stored_result = mewo_session_render(session, surface, (uint64_t)(time_ms * 1e6));
}

EMSCRIPTEN_KEEPALIVE
double session_get_time(mewo_session_t* session) {
  return (double)mewo_session_get_time(session) / 1e6;
}

EMSCRIPTEN_KEEPALIVE
void session_set_paused(mewo_session_t* session, bool paused) {
  mewo_session_set_paused(session, paused);
}

EMSCRIPTEN_KEEPALIVE
bool session_is_paused(mewo_session_t* session) { return mewo_session_is_paused(session); }

EMSCRIPTEN_KEEPALIVE
void session_reset(mewo_session_t* session) { mewo_session_reset(session); }

EMSCRIPTEN_KEEPALIVE
double session_get_pass_status_revision(mewo_session_t* session) {
  return (double)mewo_session_get_pass_status_revision(session);
}

EMSCRIPTEN_KEEPALIVE
mewo_pass_compile_status_t session_get_pass_compile_status(
  mewo_session_t* session,
  mewo_pass_id_t pass_id
) {
  mewo_pass_compile_status_t status = MEWO_PASS_COMPILE_STATUS_PENDING;
  stored_result = mewo_session_get_pass_compile_status(session, pass_id, &status);
  return status;
}

EMSCRIPTEN_KEEPALIVE
double session_get_pass_compile_duration(mewo_session_t* session, mewo_pass_id_t pass_id) {
  uint64_t duration_ns = 0;
  stored_result = mewo_session_get_pass_compile_duration(session, pass_id, &duration_ns);
  return (double)duration_ns * 1e-6;
}

/** The number of diagnostics from the pass' last completed compile. */
EMSCRIPTEN_KEEPALIVE
uint32_t session_get_pass_diagnostic_count(mewo_session_t* session, mewo_pass_id_t pass_id) {
  uint32_t count = 0;
  stored_result = mewo_session_get_pass_diagnostic_count(session, pass_id, &count);
  return count;
}

/**
 * Selects one diagnostic as the current diagnostic, readable through the
 * getters until the next render.
 */
EMSCRIPTEN_KEEPALIVE
void session_select_pass_diagnostic(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  uint32_t index
) {
  stored_result = mewo_session_get_pass_diagnostic(session, pass_id, index, &current_diagnostic);
}

EMSCRIPTEN_KEEPALIVE
int diagnostic_severity(void) { return (int)current_diagnostic.severity; }

EMSCRIPTEN_KEEPALIVE
const char* diagnostic_message(void) { return to_c_string(current_diagnostic.message); }

EMSCRIPTEN_KEEPALIVE
int diagnostic_source(void) { return (int)current_diagnostic.source; }

/** 1-based, 0 if the diagnostic has no position. */
EMSCRIPTEN_KEEPALIVE
double diagnostic_line(void) { return (double)current_diagnostic.line; }

/** 1-based, in UTF-8 bytes, 0 if the diagnostic has no position. */
EMSCRIPTEN_KEEPALIVE
double diagnostic_column(void) { return (double)current_diagnostic.column; }
