#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace mewo::gfx {

struct CompilationDiagnostic {
  std::string message;
  std::string_view type_name; ///< Will reference statically-allocated string.
  uint64_t line_num = {};
  uint64_t line_pos = {};
  std::string highlight;
};

}
