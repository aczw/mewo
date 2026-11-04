#pragma once

#include "line.hpp"

#include <optional>
#include <string>
#include <utility>

namespace mewo {

enum class Severity { kError, kWarning, kInfo };

struct Diagnostic {
  Severity severity;
  std::string message;
  std::optional<std::pair<RelativeLine, LineCol>> line;
};

}  // namespace mewo
