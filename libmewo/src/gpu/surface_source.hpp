#pragma once

#include "gpu/types.hpp"

#include <webgpu/webgpu.h>

#include <variant>

namespace mewo {

struct MetalLayer {
  void* layer;
};

struct WindowsHwnd {
  void* hinstance;
  void* hwnd;
};

using SurfaceSource = std::variant<MetalLayer, WindowsHwnd>;

Surface create_surface_from_source(const Instance& instance, const SurfaceSource& source);

}  // namespace mewo
