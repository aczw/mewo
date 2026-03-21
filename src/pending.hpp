#pragma once

namespace mewo {

/// Collects pending operations to be applied and processed in the next frame.
struct Pending {
  bool quit = false;
};

}
