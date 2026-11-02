#pragma once

#include "core/gpu/types.hpp"

#include <webgpu/webgpu.h>

namespace mewo::impl {

class SurfaceBlit {
 public:
  SurfaceBlit() = default;

  SurfaceBlit(SurfaceBlit&&) = default;
  SurfaceBlit& operator=(SurfaceBlit&&) = default;
  SurfaceBlit(const SurfaceBlit&) = delete;
  SurfaceBlit& operator=(const SurfaceBlit&) = delete;

  const gpu::RenderPipeline& render_pipeline() const { return render_pipeline_; }

  /**
   * We only learn what the surface format is after configuring it, so we have to
   * defer creating the render pipeline at construction.
   */
  void prepare(
    const gpu::Device& device,
    const gpu::BindGroupLayout& bind_group_layout,
    WGPUTextureFormat surface_format
  );

 private:
  gpu::RenderPipeline render_pipeline_;
  WGPUTextureFormat current_surface_format_ = WGPUTextureFormat_Undefined;
};

}  // namespace mewo::impl
