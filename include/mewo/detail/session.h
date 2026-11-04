#ifndef MEWO_SESSION_H
#define MEWO_SESSION_H

#include <mewo/detail/pass.h>
#include <mewo/detail/types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mewo_session_descriptor {
  /** This is a reference to a project that must outlive the session. */
  mewo_project_t* project;
  /** Cannot be zero, otherwise will be rejected with `MEWO_ERROR_INVALID_ARGUMENT`. */
  uint32_t render_width;
  /** Cannot be zero, otherwise will be rejected with `MEWO_ERROR_INVALID_ARGUMENT`. */
  uint32_t render_height;
} mewo_session_descriptor_t;

/**
 * Creates and allocates a session from a given project. `*out` is set to `NULL` on failure.
 *
 * @param runtime Pointer to the runtime.
 * @param desc Pointer to the descriptor containing configuration options.
 * @param out Out parameter to the pointer where the new session will be stored.
 * @return The result of the operation. `MEWO_ERROR_NOT_READY` is returned if
 * the device isn't ready.
 */
mewo_result_t mewo_session_create(
  mewo_runtime_t* runtime,
  const mewo_session_descriptor_t* desc,
  mewo_session_t** out
);

/**
 * Destroys the session. Must be destroyed before its project and runtime.
 *
 * @param session Pointer to the session. If `NULL` is passed, nothing happens.
 */
void mewo_session_destroy(mewo_session_t* session);

/**
 * Sets the new dimensions that the session should render at.
 *
 * @param session Pointer to the session.
 * @param new_width Width of the render. Cannot be zero.
 * @param new_height Height of the render. Cannot be zero.
 * @return `MEWO_SUCCESS` or `MEWO_ERROR_INVALID_ARGUMENT` is invalid dimensions are passed in.
 */
mewo_result_t mewo_session_set_render_size(
  mewo_session_t* session,
  uint32_t new_width,
  uint32_t new_height
);

/**
 * Attempt to render one frame of the project at the given time.
 *
 * @param session Pointer to the session.
 * @param surface Pointer to the surface to render to. Can be `NULL`, meaning that the session is
 * rendering in headless mode.
 * @param current_time The current time of the host, in nanoseconds. Should be monotonic.
 * Time going backwards is treated as no time passing.
 * @return The result of the operation. Note that `MEWO_SUCCESS` is returned for skipped or dropped
 * frames as well.
 */
mewo_result_t mewo_session_render(
  mewo_session_t* session,
  mewo_surface_t* surface,
  uint64_t current_time
);

/**
 * Each time the compile status of a pass changes, the revision count is incremented.
 * Use this to check if the status of any pass in the session has changed.
 *
 * @param session Pointer to the session.
 * @return The current revision count of the session.
 */
uint64_t mewo_session_get_status_revision(mewo_session_t* session);

/**
 * If the pass status is currently an error, `mewo_pass_compile_status_t` will contain a
 * message that is valid until the next call to `mewo_session_render()`.
 *
 * @param session Pointer to the session.
 * @param pass_id The ID of the pass to check the status for.
 * @param out Out parameter where the pass status will be stored.
 * @return `MEWO_ERROR_INVALID_ARGUMENT` if the pass ID is invalid, and `MEWO_SUCCESS` otherwise.
 */
mewo_result_t mewo_session_get_pass_compile_status(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  mewo_pass_compile_status_t* out
);

#ifdef __cplusplus
}
#endif

#endif
