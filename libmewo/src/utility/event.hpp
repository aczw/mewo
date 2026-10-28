#pragma once

#include <string>
#include <variant>

namespace mewo {

struct AdapterRequestFailed {
  enum class Reason { kError, kUnavailable };

  Reason reason;
  std::string message;
};

using Event = std::variant<AdapterRequestFailed>;

}  // namespace mewo
