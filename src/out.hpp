#pragma once

#include "gfx/frame_context.hpp"

namespace mewo {

/// Visual output seen on the screen.
class Out {
  public:
  void record(const gfx::FrameContext& frame_ctx) const;
};

}
