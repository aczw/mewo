#pragma once

#include <cstdint>
#include <filesystem>
#include <variant>

namespace mewo::event {

struct QuitRequest {};

struct ChooseFolderRequest {
  enum class Reason : uint8_t {
    ProjectOpen,
    ProjectSaveAs,
  } reason = Reason::ProjectOpen;
};

struct ProjectOpenRequest {
  std::filesystem::path directory;
};

struct ProjectSaveAsRequest {
  std::filesystem::path directory;
};

using Event = std::variant<
  const QuitRequest,
  const ChooseFolderRequest,
  const ProjectOpenRequest,
  const ProjectSaveAsRequest
>;

}  // namespace mewo::event
