#pragma once

#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"

#include <array>

namespace mewo::impl {

struct OutPass {
  struct alignas(16) Uniforms {
    std::array<float, 2> resolution = {};
    float time = {};
    float delta_time = {};
  };

  static_assert(sizeof(Uniforms) == 16);

  gpu::Buf unif_buf;
  gpu::BindGroup bg;
  gpu::RenderPipeline rpl;
  gpu::TexView view;
};

gpu::TexView create_out_tex(const gpu::Device& device, Dimensions render_size);
OutPass create_out_pass(const gpu::Device& device, Dimensions render_size);

}  // namespace mewo::impl
