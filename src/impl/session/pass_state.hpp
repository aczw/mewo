#pragma once

#include "core/gpu/types.hpp"
#include "impl/project/pass.hpp"

#include <string>

namespace mewo::impl {

class PassState {
 public:
  PassState(
    const gpu::Device& device,
    const gpu::BindGroupLayout& common_unifs_bgl,
    const gpu::BindGroupLayout& prev_bgl,
    const Pass& pass
  );

  PassState(PassState&&) = default;
  PassState& operator=(PassState&&) = default;
  PassState(const PassState&) = delete;
  PassState& operator=(const PassState&) = delete;

  const gpu::RenderPipeline& render_pipeline() const { return render_pipeline_; }

  const std::string& label() const { return label_; }

 private:
  gpu::RenderPipeline render_pipeline_;
  std::string label_;
};

}  // namespace mewo::impl
