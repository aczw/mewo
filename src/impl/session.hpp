#pragma once

#include "impl/common/dimensions.hpp"
#include "impl/project.hpp"
#include "impl/runtime.hpp"
#include "impl/session/out_pass.hpp"
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

  void set_render_size(Dimensions render_size);

  void render(impl::Surface* surface, uint64_t current_time);

 private:
  Session(
    impl::Runtime& runtime,
    impl::Project& project,
    Dimensions render_size,
    OutPass&& out_pass
  );

  impl::Runtime& runtime_;
  [[maybe_unused]] impl::Project& project_;
  Dimensions render_size_;

  std::optional<uint64_t> previous_time_;
  uint64_t elapsed_;

  OutPass out_pass_;
};

}  // namespace mewo::impl

struct mewo_session {
  std::unique_ptr<mewo::impl::Session> impl;
};
