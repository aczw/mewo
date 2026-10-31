#pragma once

#include "impl/common/dimensions.hpp"
#include "impl/project.hpp"
#include "impl/runtime.hpp"
#include "impl/surface.hpp"

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

  void render(impl::Surface* surface, uint64_t host_time);

 private:
  Session(impl::Runtime& runtime, impl::Project& project, Dimensions render_size)
      : runtime_(runtime), project_(project), render_size_(render_size) {}

  impl::Runtime& runtime_;
  impl::Project& project_;
  Dimensions render_size_;
};

}  // namespace mewo::impl

struct mewo_session {
  std::unique_ptr<mewo::impl::Session> impl;
};
