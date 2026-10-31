#include "impl/session.hpp"

#include "impl/common/dimensions.hpp"

#include <memory>
#include <optional>

namespace mewo::impl {

std::optional<std::unique_ptr<Session>> Session::create(
  impl::Runtime& runtime,
  impl::Project& project,
  Dimensions render_size
) {
  // Device is not ready yet, cannot create a session
  if (!runtime.device())
    return std::nullopt;

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Session>(new Session(runtime, project, render_size));
}

}  // namespace mewo::impl
