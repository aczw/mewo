#pragma once

#include <cstdint>

namespace mewo {

struct State {
  uint32_t curr_viewport_window_width = 0; ///< Cached every frame.
  uint32_t prev_viewport_window_width = 0; ///< Cached every frame.

  bool should_quit = false;
};

}
