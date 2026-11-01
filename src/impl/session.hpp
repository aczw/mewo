#pragma once

#include "core/gpu/types.hpp"
#include "core/pinned.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/project.hpp"
#include "impl/runtime.hpp"
#include "impl/session/pass_state.hpp"
#include "impl/session/surface_blit.hpp"
#include "impl/surface.hpp"

#include <array>
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

  void set_render_size(Dimensions render_size);

  void render(impl::Surface* surface, uint64_t current_time);

 private:
  struct alignas(16) CommonUnifs {
    std::array<float, 2> resolution = {};
    float time = {};
    float delta_time = {};
  };

  static_assert(sizeof(CommonUnifs) == 16);

  Session(
    impl::Runtime& runtime,
    impl::Project& project,
    Dimensions render_size,
    gpu::Buf&& common_unifs_buf,
    gpu::BindGroupLayout&& common_unifs_bgl,
    gpu::BindGroup&& common_unifs_bg,
    std::vector<PassState>&& passes,
    SurfaceBlit&& blit
  );

  impl::Runtime& runtime_;
  [[maybe_unused]] impl::Project& project_;
  Dimensions render_size_;

  std::optional<uint64_t> previous_time_;
  uint64_t elapsed_;

  gpu::Buf common_unifs_buf_;
  gpu::BindGroupLayout common_unifs_bgl_;
  gpu::BindGroup common_unifs_bg_;

  std::vector<PassState> passes_;
  SurfaceBlit blit_;
};

}  // namespace mewo::impl

struct mewo_session {
  std::unique_ptr<mewo::impl::Session> impl;
};
