#include "project.hpp"

#include "exception.hpp"
#include "query.hpp"

#include <filesystem>
#include <fstream>
#include <print>

namespace mewo {

Project::Project(const std::filesystem::path& folder_to_open)
{
  namespace fs = std::filesystem;

  auto path_str = fs::weakly_canonical(folder_to_open).string();

  if (!fs::exists(folder_to_open))
    throw Exception("Project folder \"{}\" does not exist", path_str);

  auto absolute = fs::absolute(folder_to_open);
  path_str = absolute.string();

  if (!fs::is_directory(absolute))
    throw Exception("\"{}\" is not a folder", path_str);

  auto mewo_folder = absolute / MEWO_FOLDER_NAME;

  if (!fs::exists(mewo_folder) || !fs::is_directory(mewo_folder))
    throw Exception("\"{}\" either does not exist or is not a folder", mewo_folder.string());

  root_directory_ = absolute;
  name_ = root_directory_.filename().string();
}

Project Project::save_as(const std::filesystem::path& directory, std::string_view code)
{
  namespace fs = std::filesystem;

  auto path_str = directory.string();

  if (!fs::exists(directory))
    throw Exception("\"{}\" does not exist", path_str);

  auto absolute_path = fs::absolute(directory);
  path_str = absolute_path.string();

  if (!fs::is_directory(absolute_path))
    throw Exception("\"{}\" is not a folder", path_str);

  if (!fs::is_empty(absolute_path))
    throw Exception("\"{}\" is not empty, abandoning", path_str);

  // Create Mewo folder
  if (!fs::create_directory(absolute_path / Project::MEWO_FOLDER_NAME)) {
    throw Exception(
        "Failed to create {1} folder at \"{0}/{1}\"", path_str, Project::MEWO_FOLDER_NAME);
  }

  if constexpr (query::is_debug())
    std::println("Saved as project \"{}\"", path_str);

  auto new_project = Project(absolute_path, SkipPathValidationTag {});
  new_project.save(code);

  return new_project;
}

void Project::save(std::string_view code) const
{
  auto file_location = shader_file_location();

  if (std::ofstream shader_file(file_location); !shader_file || !shader_file.is_open()) {
    throw Exception("Failed to access \"{}\" for writing", file_location.string());
  } else {
    shader_file << code;
  }
}

Project::Project(const std::filesystem::path& existing_directory, SkipPathValidationTag)
    : root_directory_(std::filesystem::absolute(existing_directory))
    , name_(root_directory_.filename().string())
{
}

}
