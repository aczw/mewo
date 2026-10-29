#ifndef MEWO_TYPES_H
#define MEWO_TYPES_H

#include <limits.h>
#include <stddef.h>

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
  MEWO_ERROR_INVALID_ARGUMENT = -3,
  MEWO_RESULT_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_result_t;

/**
 * Other languages that call this API may not have strings that are null-terminated, so
 * to avoid unnecessary copies we use our own string type.
 *
 * This library guarantees that all functions returning a string view will carry an explicit
 * length (not `MEWO_STRING_VIEW_SENTINEL`), but it may not be null-terminated.
 */
typedef struct mewo_string_view {
  const char* data;
  size_t length;
} mewo_string_view_t;

/**
 * Used to differentiate a string view that has an explicit length from one that
 * we know is null-terminated. There are 5 variants possible:
 *
 * - { NULL, MEWO_STRING_VIEW_SENTINEL }: essentially a null pointer.
 * - { real_ptr, MEWO_STRING_VIEW_SENTINEL }: a null-terminated string view.
 * - { real_ptr, any_length }: a string view with an explicit length.
 * - { any_ptr, 0 }: an empty string view ("").
 * - { NULL, any_length }: this is invalid.
 */
#define MEWO_STRING_VIEW_SENTINEL SIZE_MAX

#ifdef __cplusplus
}
#endif

#endif
