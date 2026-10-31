#ifndef MEWO_RUNTIME_H
#define MEWO_RUNTIME_H

#include <mewo/detail/event.h>
#include <mewo/detail/types.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates and allocates the runtime. On failure, `*out` is set to `NULL`.
 *
 * @param out Out parameter where a pointer to the runtime will be stored.
 * @return Returns whether the operation was successful.
 */
mewo_result_t mewo_runtime_create(mewo_runtime_t** out);

/**
 * Destroys and ends the runtime. Surfaces and sessions created from this runtime
 * must be destroyed first.
 *
 * @param runtime Pointer to the runtime. If `NULL` is passed in, nothing happens.
 */
void mewo_runtime_destroy(mewo_runtime_t* runtime);

/**
 * Pops the next event from the event queue. Strings in the event only stay valid
 * until the next event poll operation.
 *
 * @param runtime Pointer to the runtime.
 * @param out Pointer to the location where the popped event will be stored, if any.
 * @return True if an event was popped from the queue, and false if the queue was empty.
 */
bool mewo_runtime_poll_event(mewo_runtime_t* runtime, mewo_event_t* out);

/**
 * Call this every frame so the runtime can process pending async work and queue any new
 * resulting events.
 *
 * @param runtime Pointer to the runtime.
 */
void mewo_runtime_tick(mewo_runtime_t* runtime);

/**
 * Requests a GPU device. This operation is async, so check via the `MEWO_EVENT_DEVICE_READY`
 * and `MEWO_EVENT_DEVICE_FAILED` events.
 *
 * @param runtime Pointer to the runtime.
 * @param surface Pointer to the optional surface to present to, or `NULL` for headless use.
 */
void mewo_runtime_request_device(mewo_runtime_t* runtime, mewo_surface_t* surface);

#ifdef __cplusplus
}
#endif

#endif
