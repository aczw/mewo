#pragma once

#include "core/gpu/types.hpp"
#include "impl/surface/source.hpp"

#include <cstdint>
#include <memory>
#include <optional>

namespace mewo::impl {

class Surface {
 public:
  static std::optional<std::unique_ptr<Surface>> create(
    const gpu::Instance& instance,
    uint32_t initial_width,
    uint32_t initial_height,
    const SurfaceSource& source
  );

  Surface(Surface&&) = delete;
  Surface& operator=(Surface&&) = delete;
  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;

  void resize(uint32_t new_width, uint32_t new_height);

  const gpu::Surface& surface() const { return surface_; }

 private:
  explicit Surface(gpu::Surface&& surface, uint32_t initial_width, uint32_t initial_height)
      : surface_(std::move(surface)),
        width_(initial_width),
        height_(initial_height),
        needs_configure_(true) {}

  gpu::Surface surface_;
  uint32_t width_;
  uint32_t height_;
  bool needs_configure_;
};

}  // namespace mewo::impl

struct mewo_surface {
  std::unique_ptr<mewo::impl::Surface> impl;
};
