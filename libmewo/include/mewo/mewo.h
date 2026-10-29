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

/**
 * Destroys and ends the runtime.
 *
 * @param runtime Pointer to the runtime. If `NULL`, nothing happens.
 */
void mewo_runtime_destroy(mewo_runtime_t* runtime);

typedef enum mewo_event_type {
  MEWO_EVENT_NONE = 0,
  MEWO_EVENT_DEVICE_READY,
  MEWO_EVENT_DEVICE_FAILED,
  MEWO_EVENT_DEVICE_LOST,
  MEWO_EVENT_GPU_ERROR,
  MEWO_EVENT_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_event_type_t;

typedef enum mewo_device_failure {
  /** Something went wrong while requesting the device. */
  MEWO_DEVICE_FAILURE_ERROR = 0,
  /** No suitable GPU found, or WebGPU is unsupported. */
  MEWO_DEVICE_FAILURE_UNAVAILABLE,
  MEWO_DEVICE_FAILURE_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_device_failure_t;

/**
 * Maps to the corresponding `WGPUErrorType` value.
 */
typedef enum mewo_gpu_error {
  MEWO_GPU_ERROR_VALIDATION = 0,
  MEWO_GPU_ERROR_OUT_OF_MEMORY,
  MEWO_GPU_ERROR_INTERNAL,
  MEWO_GPU_ERROR_UNKNOWN,
  MEWO_GPU_ERROR_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_gpu_error_t;

/**
 * Event string messages, if any, always carry a real length.
 */
typedef struct mewo_event {
  mewo_event_type_t type;

  union {
    struct {
      mewo_device_failure_t reason;
      mewo_string_view_t message;
    } device_failed;

    struct {
      mewo_string_view_t message;
    } device_lost;

    struct {
      mewo_gpu_error_t type;
      mewo_string_view_t message;
    } gpu_error;

    uint8_t reserved[32];
  } variant;
} mewo_event_t;

/**
 * Pops the next event from the event queue. Strings in the event only stay valid
 * until the next event poll operation.
 *
 * @param runtime Pointer to the runtime.
 * @param out Pointer to the location where the popped event will be stored, if any.
 * @return True if an event was popped from the queue, and false if the queue was empty.
 */
bool mewo_runtime_poll_event(mewo_runtime_t* runtime, mewo_event_t* out);

void mewo_runtime_tick(mewo_runtime_t* runtime);

void mewo_runtime_request_device(mewo_runtime_t* runtime);

#ifdef __cplusplus
}
#endif

#endif
