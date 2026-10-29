#pragma once

#include "core/gpu_types.hpp"

#include <cstdint>
#include <memory>
#include <optional>

namespace mewo {

struct MetalLayer {
  void* layer;
};

struct Win32 {
  void* hinstance;
  void* hwnd;
};

class SurfaceImpl {
 public:
  using Source = std::variant<MetalLayer, Win32>;

  static std::optional<std::unique_ptr<SurfaceImpl>> create(
    const Instance& instance,
    uint32_t initial_width,
    uint32_t initial_height,
    const Source& source
  );

  SurfaceImpl(SurfaceImpl&&) = delete;
  SurfaceImpl& operator=(SurfaceImpl&&) = delete;
  SurfaceImpl(const SurfaceImpl&) = delete;
  SurfaceImpl& operator=(const SurfaceImpl&) = delete;

  void resize(uint32_t new_width, uint32_t new_height);

  const Surface& surface() const { return surface_; }

 private:
  explicit SurfaceImpl(Surface&& surface, uint32_t initial_width, uint32_t initial_height)
      : surface_(std::move(surface)),
        width_(initial_width),
        height_(initial_height),
        needs_configure_(false) {}

  Surface surface_;
  uint32_t width_;
  uint32_t height_;
  bool needs_configure_;
};

}  // namespace mewo
