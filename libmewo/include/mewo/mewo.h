#ifndef MEWO_H_
#define MEWO_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type that represents the runtime. This owns the WebGPU instance, device,
 * and everything in between.
 */
typedef struct mewo_runtime mewo_runtime_t;

/**
 * Opaque type that represents a session. This can be thought of as an "instance" of an
 * open project that is currently running.
 */
typedef struct mewo_session mewo_session_t;

/**
 * Opaque type that represents a project. This is a document that stores the passes, sources,
 * and settings that encompass a project. Serializable by the host.
 */
typedef struct mewo_project mewo_project_t;

/**
 * Opaque type that represents a window surface. This is the surface that the render will
 * present to each frame.
 */
typedef struct mewo_surface mewo_surface_t;

/**
 * Sizes the enum such that the type the compiler chooses is compatible with `INT_MAX`.
 */
#define MEWO_ENUM_FORCE_INT INT_MAX

/**
 * Represents all possible results from operations.
 */
typedef enum mewo_result {
  /** Everything succeeded */
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

/**
 * Retrieves what version of Mewo is currently running.
 *
 * @return Pointer to the null-terminated version string.
 */
const char* mewo_get_version(void);

#ifdef __cplusplus
}
#endif

#endif
