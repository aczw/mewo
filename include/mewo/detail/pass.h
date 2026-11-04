#ifndef MEWO_PASS_H
#define MEWO_PASS_H

#include <mewo/detail/types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pass IDs start at 1, so an unknown or invalid pass ID uses this value.
 */
#define MEWO_PASS_ID_NONE 0

/**
 * An ID used to refer to a pass within a project.
 */
typedef uint32_t mewo_pass_id_t;

typedef enum mewo_pass_status_type {
  MEWO_PASS_STATUS_RUNNING = 0,
  MEWO_PASS_STATUS_COMPILING,
  MEWO_PASS_STATUS_COMPILE_SUCCESS,
  MEWO_PASS_STATUS_COMPILE_ERROR,
  MEWO_PASS_STATUS_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_pass_status_type_t;

typedef enum mewo_pass_compile_error {
  MEWO_PASS_COMPILE_ERROR_VALIDATION = 0,
  MEWO_PASS_COMPILE_ERROR_INTERNAL,
  MEWO_PASS_COMPILE_ERROR_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_pass_compile_error_t;

typedef struct mewo_pass_status {
  mewo_pass_status_type_t type;

  union {
    struct {
      mewo_pass_compile_error_t type;
      mewo_string_view_t message;
    } compile_error;

    uint8_t reserved[32];
  } variant;
} mewo_pass_status_t;

#ifdef __cplusplus
}
#endif

#endif
