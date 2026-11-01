#pragma once

#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/project/pass.hpp"

#include <string>

namespace mewo::impl {

gpu::TexView create_tex(const gpu::Device& device, Dimensions render_size);

class PassState {
 public:
  PassState(
    const gpu::Device& device,
    const gpu::BindGroupLayout& common_unifs_bgl,
    const Pass& pass,
    Dimensions render_size
  );

  PassState(PassState&&) = default;
  PassState& operator=(PassState&&) = default;
  PassState(const PassState&) = delete;
  PassState& operator=(const PassState&) = delete;

  const gpu::RenderPipeline& render_pipeline() const { return render_pipeline_; }

  const gpu::TexView& view() const { return view_; }

  const std::string& label() const { return label_; }

  void resize(const gpu::Device& device, Dimensions new_render_size) {
    view_ = create_tex(device, new_render_size);
  }

 private:
  Pass::Id id_;

  gpu::RenderPipeline render_pipeline_;
  gpu::TexView view_;
  std::string label_;
};

}  // namespace mewo::impl
