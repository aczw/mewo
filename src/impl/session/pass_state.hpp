#pragma once

#include "core/gpu/types.hpp"
#include "impl/project/pass.hpp"

#include <optional>
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

  Pass::Id pass_id() const { return pass_id_; }

  const gpu::RenderPipeline& render_pipeline() const { return render_pipeline_; }

  const std::string& pass_label() const { return pass_label_; }

  void rebuild_render_pipeline(const gpu::Device& device, const Pass& pass);

  bool is_stale(const Pass& pass) const;

 private:
  Pass::Id pass_id_;

  gpu::PipelineLayout pipeline_layout_;
  gpu::RenderPipeline render_pipeline_;

  std::string pass_label_;
  std::string shader_module_label_;
  std::string render_pipeline_label_;

  /**
   * Revision number of the source the current pipeline was built from. Empty if never built.
   */
  std::optional<Pass::Revision> revision_;
};

}  // namespace mewo::impl
