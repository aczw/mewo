#pragma once

#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/project.hpp"
#include "impl/runtime.hpp"
#include "impl/surface.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>

namespace mewo::impl {

class Session {
 public:
  static std::optional<std::unique_ptr<Session>> create(
    impl::Runtime& runtime,
    impl::Project& project,
    Dimensions render_size
  );

  Session(Session&&) = delete;
  Session& operator=(Session&&) = delete;
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;

  void set_render_size(Dimensions render_size);

  void render(impl::Surface* surface, uint64_t current_time);

 private:
  struct alignas(16) Uniforms {
    std::array<float, 2> resolution = {};
    float time = {};
    float delta_time = {};
  };

  static_assert(sizeof(Uniforms) == 16);

  Session(
    impl::Runtime& runtime,
    impl::Project& project,
    Dimensions render_size,
    gpu::Buf&& out_unif_buf,
    gpu::BindGroup&& out_bg,
    gpu::RenderPipeline&& out_rpl,
    gpu::TexView&& out_view
  );

  impl::Runtime& runtime_;
  [[maybe_unused]] impl::Project& project_;
  Dimensions render_size_;

  std::optional<uint64_t> previous_time_;
  uint64_t elapsed_;

  gpu::Buf out_unif_buf_;
  gpu::BindGroup out_bg_;
  gpu::RenderPipeline out_rpl_;
  gpu::TexView out_view_;
};

}  // namespace mewo::impl

struct mewo_session {
  std::unique_ptr<mewo::impl::Session> impl;
};
