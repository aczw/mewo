#pragma once

#include <filesystem>
#include <optional>

namespace mewo {

/// Collects pending operations to be applied and processed in the next frame.
struct Pending {
  bool quit = false;
  std::optional<std::filesystem::path> project_open;
};

}
