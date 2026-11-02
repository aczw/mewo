#pragma once

#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"

#include <array>

namespace mewo::impl {

class PassTargets {
 public:
  static constexpr WGPUTextureFormat kTexFormat = WGPUTextureFormat_RGBA16Float;

  PassTargets(const gpu::Device& device, Dimensions render_size);

  PassTargets(PassTargets&&) = default;
  PassTargets& operator=(PassTargets&&) = default;
  PassTargets(const PassTargets&) = delete;
  PassTargets& operator=(const PassTargets&) = delete;

  void resize(const gpu::Device& device, Dimensions new_render_size);

  /**
   * Clears view B to black. Always encoded before the first pass.
   */
  void clear(const gpu::CmdEncoder& cmd_encoder) const;

  const gpu::BindGroupLayout& bind_group_layout() const { return bind_group_layout_; }

  const gpu::TexView& target_view(size_t pass_idx) const { return views_[pass_idx % 2]; }

  const gpu::BindGroup& prev_bind_group(size_t pass_idx) const {
    return bind_groups_[(pass_idx + 1) % 2];
  }

 private:
  /**
   * [0] = A, [1] = B
   */
  std::array<gpu::TexView, 2> views_;
  gpu::Sampler sampler_;
  gpu::BindGroupLayout bind_group_layout_;
  std::array<gpu::BindGroup, 2> bind_groups_;
};

}  // namespace mewo::impl
