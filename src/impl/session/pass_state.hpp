#pragma once

#include "core/gpu/types.hpp"
#include "core/pass.hpp"
#include "impl/runtime.hpp"
#include "impl/runtime/compile_request.hpp"

#include <optional>
#include <string>

namespace mewo::impl {

class PassState {
 public:
  enum class Status { kRunning, kCompiling, kCompileSuccess, kCompileError };

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

  Status status() const { return status_; }

  const CompileRequestError* current_compile_error() const {
    if (current_compile_error_)
      return &current_compile_error_.value();

    return nullptr;
  }

  void request_render_pipeline_build(Runtime& runtime, const Pass& pass);

  /**
   * @return True if the status of this pass state has changed, and false otherwise.
   */
  bool poll_compile_request(Runtime& runtime);

  void discard_compile_request(Runtime& runtime);

  bool is_stale(const Pass& pass) const;

 private:
  Pass::Id pass_id_;

  gpu::PipelineLayout pipeline_layout_;
  gpu::RenderPipeline render_pipeline_;

  std::string pass_label_;
  std::string shader_module_label_;
  std::string render_pipeline_label_;

  /**
   * Revision number of the source code whose pipeline was last attempted
   * to be built. Empty if never requested.
   */
  std::optional<Pass::Revision> revision_;
  std::optional<CompileRequestId> current_compile_request_id_;

  Status status_;
  std::optional<CompileRequestError> current_compile_error_;
};

}  // namespace mewo::impl
