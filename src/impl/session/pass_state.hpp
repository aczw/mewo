#pragma once

#include "core/common.hpp"
#include "core/compilation/diagnostic.hpp"
#include "core/gpu/types.hpp"
#include "core/pass.hpp"
#include "impl/runtime.hpp"
#include "impl/runtime/compile_request.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace mewo::impl {

class PassState {
 public:
  struct Pending {
    uint64_t start_ns = 0;
  };

  struct Success {
    uint64_t duration_ns;
  };

  struct Error {
    uint64_t duration_ns;
  };

  using CompileStatus = std::variant<Pending, Success, Error>;

  PassState(
    const gpu::Device& device,
    const gpu::BindGroupLayout& unifs_bgl,
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

  const CompileStatus& compile_status() const { return compile_status_; }

  const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }

  void request_render_pipeline_build(
    Runtime& runtime,
    const Pass& pass,
    std::string_view common_source,
    Revision common_revision,
    uint64_t start_ns
  );

  /** @return True if the status of this pass state has changed, and false otherwise. */
  bool poll_compile_request(Runtime& runtime, uint64_t end_ns);

  void discard_compile_request(Runtime& runtime);

  bool is_stale(const Pass& pass, Revision common_revision) const;

 private:
  Pass::Id pass_id_;
  std::string pass_label_;

  gpu::PipelineLayout pipeline_layout_;
  gpu::RenderPipeline render_pipeline_;

  std::optional<Revision> pass_revision_;
  std::optional<Revision> common_revision_;

  std::optional<CompileRequestId> current_compile_request_id_;
  CompileStatus compile_status_;
  /** Diagnostics from the last completed compile. */
  std::vector<Diagnostic> diagnostics_;
};

}  // namespace mewo::impl
