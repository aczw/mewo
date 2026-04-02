#pragma once

#include <cstdint>
#include <filesystem>
#include <variant>

namespace mewo::event {

struct QuitRequest {};

struct ProjectOpen {
  std::filesystem::path root_dir;
};

struct ProjectSaveAs {
  std::filesystem::path root_dir;
};

struct ProjectSave {};

struct ViewportResize {
  uint32_t new_width = 0;
  uint32_t new_height = 0;
};

struct Run {};

using Event = std::variant<const QuitRequest>;

}  // namespace mewo::event
