#ifndef MEWO_SURFACE_H
#define MEWO_SURFACE_H

#include <mewo/categories/types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mewo_surface_source_type {
  MEWO_SURFACE_SOURCE_NONE = 0,
  MEWO_SURFACE_SOURCE_METAL_LAYER,
  MEWO_SURFACE_SOURCE_WINDOWS_HWND,
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
    } windows_hwnd;
  } variant;
} mewo_surface_descriptor_t;

mewo_result_t mewo_surface_create(
  mewo_runtime_t* runtime,
  const mewo_surface_descriptor_t* desc,
  mewo_surface_t** out
);

/**
 * Destroys the window surface.
 *
 * @param surface Pointer to the surface. If `NULL` is passed in, nothing happens.
 */
void mewo_surface_destroy(mewo_surface_t* surface);

void mewo_surface_resize(mewo_surface_t* surface, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif

#endif
