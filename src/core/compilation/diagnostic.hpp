#pragma once

#include "core/compilation/line.hpp"

#include <optional>
#include <string>

namespace mewo {

enum class Severity { kError, kWarning, kInfo };

struct Diagnostic {
  struct Location {
    RelativeLine line;
    LineCol column;
  };

  Severity severity;
  std::string message;
  std::optional<Location> location;
};

}  // namespace mewo
