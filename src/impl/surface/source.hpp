#pragma once

#include <variant>

namespace mewo::impl {

struct MetalLayer {
  void* layer;
};

struct Win32 {
  void* hinstance;
  void* hwnd;
};

using SurfaceSource = std::variant<MetalLayer, Win32>;

}  // namespace mewo::impl
