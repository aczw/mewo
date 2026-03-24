#include "io.hpp"

#include "exception.hpp"
#include "project.hpp"
#include "query.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <string_view>

namespace mewo::io {

static constexpr std::string_view WGSL_FILE_EXTENSION = ".wgsl";

std::string read_file(const std::filesystem::path& file_path)
{
  namespace fs = std::filesystem;

  auto path_str = file_path.string();

  if (!fs::exists(file_path) || !fs::is_regular_file(file_path))
    throw Exception("\"{}\" does not exist or is not regular", path_str);

  // Open in binary mode because otherwise it might do
  // weird things with newlines/carriage returns
  std::ifstream file(file_path, std::ios::in | std::ios::binary);

  if (!file || !file.is_open())
    throw Exception("Failed to open \"{}\"", path_str);

  auto file_size = static_cast<size_t>(fs::file_size(file_path));

  std::string source;
  source.resize(file_size);

  file.seekg(0, std::ios::beg);
  file.read(source.data(), static_cast<std::streamsize>(file_size));

  return source;
}

std::string read_wgsl_shader(const std::filesystem::path& file_path)
{
  // File path may not exist so it can only be converted weakly
  auto weak_path = std::filesystem::weakly_canonical(file_path);

  std::string source = read_file(weak_path);

  if (file_path.extension() != WGSL_FILE_EXTENSION)
    throw Exception("\"{}\" is not a WGSL shader (does not end with {})", weak_path.string(),
        WGSL_FILE_EXTENSION);

  return source;
}

Project save_as(const std::filesystem::path& folder_path, std::string_view code)
{
  namespace fs = std::filesystem;

  auto path_str = folder_path.string();

  if (!fs::exists(folder_path))
    throw Exception("\"{}\" does not exist", path_str);

  auto absolute_path = fs::absolute(folder_path);
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

  if (std::ofstream shader_file(absolute_path / Project::WGSL_SHADER_FILE_NAME);
      !shader_file || !shader_file.is_open()) {
    throw Exception(
        "Failed to access \"{}/{}\" for writing", path_str, Project::WGSL_SHADER_FILE_NAME);
  } else {
    shader_file << code;
  }

  if constexpr (query::is_debug())
    std::println("Saved as project \"{}\"", path_str);

  // TODO: this goes through another arguably redundant round of filesystem checks.
  // Some way to skip repeating the same work?
  return Project(path_str);
}

}
