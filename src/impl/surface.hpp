#pragma once

#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/surface/source.hpp"

#include <webgpu/webgpu.h>

#include <memory>
#include <optional>

namespace mewo::impl {

class Surface {
 public:
  static std::optional<std::unique_ptr<Surface>> create(
    const gpu::Instance& instance,
    Dimensions initial_dimensions,
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

  const Dimensions& dimensions() const { return dimensions_; }

  void resize(Dimensions new_dimensions);
  std::optional<gpu::TexView> acquire(const gpu::Adapter& adapter, const gpu::Device& device);

  void present() { wgpuSurfacePresent(surface_.get()); }

 private:
  Surface(gpu::Surface&& surface, Dimensions initial_dimensions)
      : surface_(std::move(surface)),
        config_(WGPU_SURFACE_CONFIGURATION_INIT),
        dimensions_(initial_dimensions),
        needs_configure_(true),
        is_invalid_(false) {}

  bool configure(const gpu::Adapter& adapter, const gpu::Device& device);

  gpu::Surface surface_;
  WGPUSurfaceConfiguration config_;

  Dimensions dimensions_;
  bool needs_configure_;
  bool is_invalid_;
};

}  // namespace mewo::impl

struct mewo_surface {
  std::unique_ptr<mewo::impl::Surface> impl;
};
