#ifndef MEWO_H_
#define MEWO_H_

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mewo_runtime mewo_runtime_t;
typedef struct mewo_session mewo_session_t;
typedef struct mewo_project mewo_project_t;
typedef struct mewo_surface mewo_surface_t;

/**
 * Sizes the enum such that the type the compiler chooses is compatible with `INT_MAX`.
 */
#define MEWO_ENUM_FORCE_INT INT_MAX

typedef enum mewo_result {
  MEWO_SUCCESS = 0,
  /** Operation is not ready yet, check back later */
  MEWO_ERROR_NOT_READY = -1,
  /** Something went wrong internally that is not the host's fault */
  MEWO_ERROR_INTERNAL = -2,
  MEWO_RESULT_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_result_t;

/**
 * Other languages that call this API may not have strings that are null-terminated, so
 * to avoid unnecessary copies we use our own string type.
 *
 * This library guarantees that all functions will return an explicit length,
 * null-terminated string view. The sentinel value is only provided for inputs from the host.
 */
typedef struct mewo_string_view {
  const char* data;
  size_t length;
} mewo_string_view_t;

/**
 * Used to differentiate a `mewo_string_view_t` that has an explicit length from one that
 * we know is null-terminated. There are 5 variants possible:
 *
 * - { NULL, MEWO_STRING_VIEW_SENTINEL }: essentially a null pointer.
 * - { real_ptr, MEWO_STRING_VIEW_SENTINEL }: a null-terminated string view.
 * - { real_ptr, any_length }: a string view with an explicit length.
 * - { any_ptr, 0 }: an empty string view ("").
 * - { NULL, any_length }: this is invalid.
 */
#define MEWO_STRING_VIEW_SENTINEL SIZE_MAX

mewo_string_view_t mewo_get_version(void);

mewo_result_t mewo_runtime_create(mewo_runtime_t** out);
void mewo_runtime_destroy(mewo_runtime_t* runtime);

typedef enum mewo_event_type {
  MEWO_EVENT_NONE = 0,
  MEWO_EVENT_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_event_type_t;

typedef struct mewo_event {
  mewo_event_type_t type;

  union {
    uint8_t reserved[64];
  } variant;
} mewo_event_t;

/**
 * Pops the next event from the event queue.
 *
 * @param runtime Pointer to the runtime.
 * @param out_event Pointer to the location where the popped event will be stored, if any.
 * @return True if an event was popped from the queue, and false if the queue was empty.
 */
bool mewo_runtime_poll_event(mewo_runtime_t* runtime, mewo_event_t* out_event);
void mewo_runtime_tick(mewo_runtime_t* runtime);

#ifdef __cplusplus
}
#endif

#endif
