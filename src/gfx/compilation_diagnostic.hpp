#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace mewo::gfx {

struct CompilationDiagnostic {
  std::string message;
  std::string_view type_name;  ///< Will reference statically-allocated string.
  uint64_t line_number = 0;
  uint64_t line_column = 0;
  std::string highlight;
};

using CompilationDiagnostics = std::vector<CompilationDiagnostic>;

}  // namespace mewo::gfx
