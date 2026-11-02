#pragma once

#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"

#include <array>
#include <cstddef>

namespace mewo::impl {

class PassTargets {
 public:
  static constexpr size_t kIdxA = 0;
  static constexpr size_t kIdxB = 1;
  static constexpr size_t kTargetCount = 2;
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
  void record_clear_pass(const gpu::CmdEncoder& cmd_encoder) const;

  const gpu::BindGroupLayout& bind_group_layout() const { return bind_group_layout_; }

  const gpu::TexView& target_view(size_t pass_idx) const { return views_[target_idx(pass_idx)]; }

  const gpu::BindGroup& prev_bind_group(size_t pass_idx) const {
    return bind_groups_[prev_idx(pass_idx)];
  }

 private:
  static constexpr size_t target_idx(size_t pass_idx) { return pass_idx % kTargetCount; }

  static constexpr size_t prev_idx(size_t pass_idx) { return (pass_idx + 1) % kTargetCount; }

  std::array<gpu::TexView, kTargetCount> views_;
  gpu::Sampler sampler_;
  gpu::BindGroupLayout bind_group_layout_;
  std::array<gpu::BindGroup, kTargetCount> bind_groups_;
};

}  // namespace mewo::impl
