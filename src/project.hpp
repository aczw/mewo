#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace mewo {

class Project {
  public:
  static constexpr std::string_view MEWO_FOLDER_NAME = ".mewo";

  Project(const std::filesystem::path& folder_to_open);

  /// Creates a new project directory and saves the newly-created project.
  static Project save_as(const std::filesystem::path& directory, std::string_view code);

  const std::filesystem::path& root_directory() const { return root_directory_; }

  const std::string& name() const { return name_; }

  std::filesystem::path shader_file_location() const { return root_directory_ / "shader.wgsl"; }

  void save(std::string_view code) const;

  private:
  struct SkipPathValidationTag { };

  Project(const std::filesystem::path& existing_directory, SkipPathValidationTag);

  std::filesystem::path root_directory_; ///< Absolute file path to root folder.
  std::string name_;
};

}
