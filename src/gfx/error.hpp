#pragma once

#include <string_view>

namespace mewo::gfx {

/// WebGPU errors can happen asynchronously and can trigger callback functions
/// at any time. We can't just throw inside the callback function because we might
/// be calling a C function, which are not exception safe.
///
/// Instead, we store that information inside this struct which we check later.
struct Error {
  std::string_view error_type;
  std::string_view message;
};

}
