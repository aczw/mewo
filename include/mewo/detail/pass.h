// IWYU pragma: private, include <mewo/mewo.h>

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

/**
 * The outcome of a pass' most recent compile. Details are stored in diagnostics
 * available via `mewo_session_get_pass_diagnostic()`.
 */
typedef enum mewo_pass_compile_status {
  MEWO_PASS_COMPILE_STATUS_PENDING = 0,
  MEWO_PASS_COMPILE_STATUS_SUCCESS,
  MEWO_PASS_COMPILE_STATUS_ERROR,
  MEWO_PASS_COMPILE_STATUS_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_pass_compile_status_t;

#ifdef __cplusplus
}
#endif

#endif
