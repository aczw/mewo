#pragma once

#include <string>
#include <variant>

namespace mewo::impl {

struct MetalLayer {
  void* layer;
};

struct Win32 {
  void* hinstance;
  void* hwnd;
};

struct HtmlCanvas {
  std::string selector;
};

using SurfaceSource = std::variant<MetalLayer, Win32, HtmlCanvas>;

}  // namespace mewo::impl
