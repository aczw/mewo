#pragma once

#include "core/gpu/types.hpp"
#include "impl/surface/source.hpp"

#include <webgpu/webgpu.h>

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

  ~Surface() {
    // `device` is set only if we've configured the surface once
    if (config_.device != nullptr)
      wgpuSurfaceUnconfigure(surface_.get());
  }

  Surface(Surface&&) = delete;
  Surface& operator=(Surface&&) = delete;
  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;

  const gpu::Surface& surface() const { return surface_; }

  void resize(uint32_t new_width, uint32_t new_height);
  std::optional<gpu::TexView> acquire(const gpu::Adapter& adapter, const gpu::Device& device);

 private:
  explicit Surface(gpu::Surface&& surface, uint32_t initial_width, uint32_t initial_height)
      : surface_(std::move(surface)),
        config_(WGPU_SURFACE_CONFIGURATION_INIT),
        width_(initial_width),
        height_(initial_height),
        needs_configure_(true),
        is_invalid_(false) {}

  bool configure(const gpu::Adapter& adapter, const gpu::Device& device);

  gpu::Surface surface_;
  WGPUSurfaceConfiguration config_;

  uint32_t width_;
  uint32_t height_;
  bool needs_configure_;
  bool is_invalid_;
};

}  // namespace mewo::impl

struct mewo_surface {
  std::unique_ptr<mewo::impl::Surface> impl;
};
