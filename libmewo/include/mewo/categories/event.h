#ifndef MEWO_EVENT_H
#define MEWO_EVENT_H

#include <mewo/categories/types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
