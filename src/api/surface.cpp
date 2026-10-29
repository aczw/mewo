#include "mewo/detail/surface.h"

#include "api/handles.hpp"
#include "api/result.hpp"
#include "core/surface_state.hpp"
#include "gpu/surface_source.hpp"

#include <mewo/mewo.h>

#include <cassert>
#include <optional>
#include <utility>

extern "C" {
mewo_result_t mewo_surface_create(
  mewo_runtime_t* runtime,
  const mewo_surface_descriptor_t* desc,
  mewo_surface_t** out
) {
  assert(runtime);
  assert(desc);
  assert(out);
  *out = nullptr;

  auto source_opt = [&desc]() -> std::optional<mewo::SurfaceSource> {
    switch (desc->type) {
      case MEWO_SURFACE_SOURCE_METAL_LAYER: {
        if (!desc->variant.metal_layer.layer) {
          return std::nullopt;
        } else {
          return mewo::MetalLayer{.layer = desc->variant.metal_layer.layer};
        }
      }

      case MEWO_SURFACE_SOURCE_WIN32: {
        if (!desc->variant.win32.hinstance || !desc->variant.win32.hwnd) {
          return std::nullopt;
        } else {
          return mewo::Win32{
            .hinstance = desc->variant.win32.hinstance,
            .hwnd = desc->variant.win32.hwnd,
          };
        }
      }

      default: return std::nullopt;
    }
  }();

  if (!source_opt)
    return MEWO_ERROR_INVALID_ARGUMENT;

  try {
    auto surface_res = mewo::SurfaceState::create(
      runtime->impl->instance(),
      desc->initial_width,
      desc->initial_height,
      std::move(source_opt.value())
    );

    if (!surface_res)
      return mewo::error_to_result(surface_res.error());

    *out = new mewo_surface_t(std::move(surface_res.value()));
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

void mewo_surface_destroy(mewo_surface_t* surface) { delete surface; }

void mewo_surface_resize(mewo_surface_t* surface, uint32_t new_width, uint32_t new_height) {
  assert(surface);
  surface->impl->resize(new_width, new_height);
}
}
