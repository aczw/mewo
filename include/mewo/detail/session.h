// IWYU pragma: private, include <mewo/mewo.h>

#ifndef MEWO_SESSION_H
#define MEWO_SESSION_H

#include <mewo/detail/pass.h>
#include <mewo/detail/project.h>
#include <mewo/detail/types.h>

#include <stdbool.h>
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
 * Gets the session's playback time (what shaders see as `mw.time`) as of the last call to
 * `mewo_session_render()`. It is 0 before the first render.
 *
 * @param session Pointer to the session.
 * @return The playback time in nanoseconds.
 */
uint64_t mewo_session_get_time(mewo_session_t* session);

/**
 * Pauses or resumes the session's playback. While paused, the session still renders (so
 * edits show up), but `mw.time` and `mw.frame_number` stay frozen and `mw.delta_time` is 0.
 *
 * Resuming continues from the paused time, without jumping forward.
 *
 * @param session Pointer to the session.
 * @param paused Whether the session should be paused.
 */
void mewo_session_set_paused(mewo_session_t* session, bool paused);

/**
 * Gets whether the session's playback is paused.
 *
 * @param session Pointer to the session.
 * @return `true` if the session is paused, `false` otherwise.
 */
bool mewo_session_is_paused(mewo_session_t* session);

/**
 * Each time the compile status or diagnostics of a pass change, the revision count is incremented.
 * Use this to check if the status of any pass in the session has changed.
 *
 * @param session Pointer to the session.
 * @return The current revision count of the session.
 */
uint64_t mewo_session_get_pass_status_revision(mewo_session_t* session);

/**
 * Gets whether the pass's most recent compile is still pending, succeeded, or failed. For the
 * reasons behind a failure (and warnings on success), use `mewo_session_get_pass_diagnostic()`.
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

/**
 * Gets how long the pass' last compile took, from the frame that started it to the frame that
 * received the result. Not available while a compile is pending.
 *
 * @param session Pointer to the session.
 * @param pass_id The ID of the pass to check its compile duration for.
 * @param out Out parameter where the compile duration is stored, in nanoseconds.
 * @return `MEWO_ERROR_INVALID_ARGUMENT` if the pass ID is invalid, `MEWO_ERROR_INVALID_OPERATION`
 * if the pass compilation is pending, and `MEWO_SUCCESS` otherwise.
 */
mewo_result_t mewo_session_get_pass_compile_duration(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  uint64_t* out
);

/**
 * Gets the number of diagnostics from the pass' last completed compile. While a recompile is
 * pending, the previous diagnostics stay available.
 *
 * A pass that failed to compile always has at least one diagnostic with error severity.
 *
 * @param session Pointer to the session.
 * @param pass_id The ID of the pass.
 * @param out Out parameter where the number of diagnostics will be stored.
 * @return `MEWO_ERROR_INVALID_ARGUMENT` if the pass ID is invalid, and `MEWO_SUCCESS` otherwise.
 */
mewo_result_t mewo_session_get_pass_diagnostic_count(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  uint32_t* out
);

/**
 * Gets one diagnostic from the pass. The message is valid until the next render.
 *
 * @param session Pointer to the session.
 * @param pass_id The ID of the pass.
 * @param index Index of the diagnostic.
 * @param out Out parameter where the diagnostic will be stored.
 * @return `MEWO_ERROR_INVALID_ARGUMENT` if the pass ID or index is invalid, and `MEWO_SUCCESS`
 * otherwise.
 */
mewo_result_t mewo_session_get_pass_diagnostic(
  mewo_session_t* session,
  mewo_pass_id_t pass_id,
  uint32_t index,
  mewo_diagnostic_t* out
);

#ifdef __cplusplus
}
#endif

#endif
