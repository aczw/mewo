#pragma once

#include "core/common.hpp"
#include "core/gpu/types.hpp"
#include "core/pinned.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/project.hpp"
#include "impl/runtime.hpp"
#include "impl/session/pass_state.hpp"
#include "impl/session/pass_targets.hpp"
#include "impl/session/surface_blit.hpp"
#include "impl/surface.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace mewo::impl {

class Session : private Pinned {
 public:
  static std::optional<std::unique_ptr<Session>> create(
    impl::Runtime& runtime,
    impl::Project& project,
    Dimensions render_size
  );

  ~Session();

  void set_render_size(Dimensions render_size);

  uint64_t elapsed() const { return elapsed_; }

  Revision pass_status_revision() const { return pass_status_revision_; }

  PassState* get_state(Pass::Id pass_id);

  void render(impl::Surface* surface, uint64_t current_time);

 private:
  Session(
    impl::Runtime& runtime,
    impl::Project& project,
    Dimensions render_size,
    gpu::Buf&& unifs_buf,
    gpu::BindGroupLayout&& unifs_bgl,
    gpu::BindGroup&& unifs_bg,
    std::vector<PassState>&& states,
    PassTargets&& targets
  );

  impl::Runtime& runtime_;
  impl::Project& project_;
  Dimensions render_size_;

  std::optional<uint64_t> previous_time_;
  uint64_t elapsed_;
  uint32_t frame_number_;

  gpu::Buf unifs_buf_;
  gpu::BindGroupLayout unifs_bgl_;
  gpu::BindGroup unifs_bg_;

  std::vector<PassState> states_;
  PassTargets targets_;
  SurfaceBlit blit_;

  /**
   * Updates whenever the status of a pass changed (compile started/finished, pass added/removed).
   */
  Revision pass_status_revision_;
  /** Checked against the `Project` structure revision to see if we need to reconcile. */
  Revision pass_structure_revision_;
};

}  // namespace mewo::impl

struct mewo_session {
  std::unique_ptr<mewo::impl::Session> impl;
};
