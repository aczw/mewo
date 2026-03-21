#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace mewo {

class Project {
  public:
  static constexpr std::string_view MEWO_FOLDER_NAME = ".mewo";
  static constexpr std::string_view WGSL_SHADER_FILE_NAME = "shader.wgsl";

  Project(const std::filesystem::path& folder_to_open);

  const std::string& name() const;
  const std::filesystem::path& root() const;

  private:
  std::string name_;
  std::filesystem::path root_; ///< Absolute file path to root folder.
};

}
