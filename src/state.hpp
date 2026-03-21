#pragma once

#include <filesystem>
#include <optional>

namespace mewo {

struct State {
  float time = 0.f;
  std::optional<std::filesystem::path> pending_project_open;
};

}
