#pragma once

#include <filesystem>
#include <optional>

namespace mewo {

/// Collects pending operations to be applied and processed in the next frame.
struct Pending {
  public:
  bool quit() const { return quit_; }

  std::optional<std::filesystem::path>& project_open() { return project_open_; }

  void request_quit() { quit_ = true; }

  void request_project_open(const std::filesystem::path& project_directory)
  {
    project_open_ = project_directory;
  }

  private:
  bool quit_ = false;
  std::optional<std::filesystem::path> project_open_;
};

}
