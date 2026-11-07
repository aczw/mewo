#ifndef MEWO_WEB_BINDINGS_H
#define MEWO_WEB_BINDINGS_H

#include <mewo/mewo.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * JS-friendly bindings over `mewo.h`. Conventions:
 *
 * - Handles are passed and returned as numbers in JS. `NULL` = 0, unsurprisingly.
 * - Functions that can fail return `NULL` or `false`, and then `mewo_web_last_result()` says why.
 * - Strings in are NULL-terminated UTF-8 (the JS `string` type).
 * - Strings out are NULL-terminated and stay valid until the next binding call that returns a
 *   string (`ccall` copies them into a JS string immediately, so JS never sees this).
 */

/** The `mewo_result_t` of the most recent failable binding call. */
int mewo_web_last_result(void);

const char* mewo_web_get_version(void);

mewo_runtime_t* mewo_web_runtime_create(void);

void mewo_web_runtime_destroy(mewo_runtime_t* runtime);

void mewo_web_runtime_tick(mewo_runtime_t* runtime);

void mewo_web_runtime_request_device(mewo_runtime_t* runtime, mewo_surface_t* surface);

/**
 * Pops one event and keeps it as the current event, readable through the `mewo_web_event_*`
 * getters. Returns its `mewo_event_type_t`, or `MEWO_EVENT_NONE` if the queue is empty.
 */
int mewo_web_runtime_poll_event(mewo_runtime_t* runtime);

/** The current event's message, or "" if it has none. */
const char* mewo_web_event_message(void);

/**
 * Initial size in device pixels.
 */
mewo_surface_t* mewo_web_surface_create_canvas(
  mewo_runtime_t* runtime,
  const char* selector,
  uint32_t width,
  uint32_t height
);

void mewo_web_surface_destroy(mewo_surface_t* surface);

/** Resizes the surface to the new size in device pixels. 0 is valid. */
void mewo_web_surface_resize(mewo_surface_t* surface, uint32_t width, uint32_t height);

mewo_project_t* mewo_web_project_create(void);

void mewo_web_project_destroy(mewo_project_t* project);

/** Passing 0 for the width and height is invalid. */
mewo_session_t* mewo_web_session_create(
  mewo_runtime_t* runtime,
  mewo_project_t* project,
  uint32_t width,
  uint32_t height
);

/** Passing 0 for the width and height is invalid. */
bool mewo_web_session_set_render_size(mewo_session_t* session, uint32_t width, uint32_t height);

bool mewo_web_session_render(mewo_session_t* session, mewo_surface_t* surface, double time_ms);

#endif
