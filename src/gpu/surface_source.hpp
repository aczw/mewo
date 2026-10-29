#pragma once

#include "gpu/types.hpp"

#include <webgpu/webgpu.h>

#include <variant>

namespace mewo {

struct MetalLayer {
  void* layer;
};

struct Win32 {
  void* hinstance;
  void* hwnd;
};

using SurfaceSource = std::variant<MetalLayer, Win32>;

Surface create_surface_from_source(const Instance& instance, const SurfaceSource& source);

}  // namespace mewo
