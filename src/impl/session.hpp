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

  void set_render_size(Dimensions render_size) { render_size_ = render_size; }

  void render(impl::Surface* surface, uint64_t current_time);

 private:
  struct alignas(16) Uniforms {
    std::array<float, 2> resolution = {};
    float time = {};
    float delta_time = {};
  };

  Session(impl::Runtime& runtime, impl::Project& project, Dimensions render_size);

  impl::Runtime& runtime_;
  impl::Project& project_;
  Dimensions render_size_;

  std::optional<uint64_t> previous_time_;
  uint64_t elapsed_;

  gpu::Buf unif_buf_;
  gpu::BindGroupLayout render_pipeline_bgl_;
  gpu::BindGroup render_pipeline_bg_;
  gpu::RenderPipeline render_pipeline_;
};

}  // namespace mewo::impl

struct mewo_session {
  std::unique_ptr<mewo::impl::Session> impl;
};
