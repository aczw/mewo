#pragma once

#include "core/gpu/types.hpp"

namespace mewo::impl {

struct BlitPass {
  gpu::BindGroupLayout bgl;
  gpu::BindGroup bg;
  gpu::RenderPipeline rpl;
  WGPUTextureFormat surface_format;
  gpu::Sampler smp;
};

gpu::BindGroup create_blit_bg(
  const gpu::Device& device,
  const gpu::BindGroupLayout& bgl,
  const gpu::TexView& source,
  const gpu::Sampler& smp
);

/**
 * We only learn what the surface format is after configuring it, so we have to defer
 * creating the render pipeline from everything else.
 */
gpu::RenderPipeline create_blit_rpl(
  const gpu::Device& device,
  const gpu::BindGroupLayout& bgl,
  WGPUTextureFormat surface_format
);

BlitPass create_blit_pass(const gpu::Device& device, const gpu::TexView& source);

}  // namespace mewo::impl
