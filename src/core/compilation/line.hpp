#pragma once

#include <cstdint>

namespace mewo {

/**
 * Line numbers start at 1.
 */
using LineNum = uint64_t;

struct RelativeLine {
  enum class Kind { kPass, kCommon, kPrelude };

  Kind kind;
  LineNum number;
};

}  // namespace mewo
