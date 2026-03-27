#pragma once

#include <cstdint>
#include <filesystem>
#include <variant>

namespace mewo::engine {

struct Quit {};

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

using Event = std::variant<Quit, ProjectOpen, ProjectSaveAs, ProjectSave, ViewportResize, Run>;

}  // namespace mewo::engine
