#ifndef MEWO_SURFACE_H
#define MEWO_SURFACE_H

#include <mewo/detail/types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mewo_surface_source_type {
  MEWO_SURFACE_SOURCE_NONE = 0,
  MEWO_SURFACE_SOURCE_METAL_LAYER,
  MEWO_SURFACE_SOURCE_WIN32,
  MEWO_SURFACE_SOURCE_FORCE_INT = MEWO_ENUM_FORCE_INT,
} mewo_surface_source_type_t;

typedef struct mewo_surface_descriptor {
  mewo_surface_source_type_t type;
  uint32_t initial_width;
  uint32_t initial_height;

  union {
    struct {
      void* layer;
    } metal_layer;

    struct {
      void* hinstance;
      void* hwnd;
    } win32;
  } variant;
} mewo_surface_descriptor_t;

/**
 * Creates and allocates the window surface. The descriptor is validated immediately, but
 * invalid native handles may only be detected on first use. `*out` is set to `NULL` on failure.
 *
 * @param runtime Pointer to the runtime.
 * @param desc Pointer to the descriptor containing the surface options.
 * @param out Out parameter where the surface will be stored if successful.
 * @return Result of the operation.
 */
mewo_result_t mewo_surface_create(
  mewo_runtime_t* runtime,
  const mewo_surface_descriptor_t* desc,
  mewo_surface_t** out
);

/**
 * Destroys the window surface. All sessions must be destroyed first.
 *
 * @param surface Pointer to the surface. If `NULL` is passed in, nothing happens.
 */
void mewo_surface_destroy(mewo_surface_t* surface);

/**
 * Requests a resize operation for the given surface.
 *
 * @param surface Pointer to the surface.
 * @param new_width New width, in pixels. 0 is a valid value.
 * @param new_height New height, in pixels. 0 is a valid value.
 */
void mewo_surface_resize(mewo_surface_t* surface, uint32_t new_width, uint32_t new_height);

#ifdef __cplusplus
}
#endif

#endif
