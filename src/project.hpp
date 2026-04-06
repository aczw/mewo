#pragma once

#include "gui/editor.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace mewo {

class Project {
 public:
  static constexpr std::string_view MEWO_FOLDER_NAME = ".mewo";
  static constexpr std::string_view PROJECT_JSON_FILE_NAME = "project.json";
  static constexpr std::string_view SCHEMA_VERSION_KEY = "schema_version";
  static constexpr int SCHEMA_VERSION_VALUE = 1;

  Project(const std::filesystem::path& folder_to_open);

  /// Creates a new project directory and saves the newly-created project.
  static Project save_as(const std::filesystem::path& directory, Editor& editor);

  const std::filesystem::path& root_directory() const { return root_directory_; }

  const std::string& name() const { return name_; }

  std::filesystem::path shader_file_location() const { return root_directory_ / "shader.wgsl"; }

  void save(Editor& editor) const;

 private:
  struct SkipPathValidationTag {};

  Project(const std::filesystem::path& existing_directory, SkipPathValidationTag);

  std::filesystem::path root_directory_;  ///< Absolute file path to root folder.
  std::string name_;
};

}  // namespace mewo
