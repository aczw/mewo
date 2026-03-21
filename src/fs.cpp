#include "fs.hpp"

#include "exception.hpp"
#include "project.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <string_view>

namespace mewo::fs {

static constexpr std::string_view WGSL_FILE_EXTENSION = ".wgsl";

std::string read_file(const std::filesystem::path& file_path)
{
  auto path_str = file_path.string();

  if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path))
    throw Exception("\"{}\" does not exist or is not regular", path_str);

  // Open in binary mode because otherwise it might do weird things with newlines/carriage returns
  std::ifstream file(file_path, std::ios::in | std::ios::binary);

  if (!file || !file.is_open())
    throw Exception("Failed to open \"{}\"", path_str);

  auto file_size = static_cast<size_t>(std::filesystem::file_size(file_path));

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

void save_as(const std::filesystem::path& folder_path, std::string_view code)
{
  // TODO: can probably take this repeated part and put it in a separate function
  auto path_str = folder_path.string();

  if (!std::filesystem::exists(folder_path)) {
    std::println("error: \"{}\" does not exist", path_str);
    return;
  }

  auto absolute_path = std::filesystem::absolute(folder_path);
  path_str = absolute_path.string();

  if (!std::filesystem::is_directory(absolute_path)) {
    std::println("error: \"{}\" is not a folder", path_str);
    return;
  }

  if (!std::filesystem::is_empty(absolute_path)) {
    std::println("error: \"{}\" is not empty, abandoning", path_str);
    return;
  }

  // Create Mewo folder
  if (!std::filesystem::create_directory(absolute_path / Project::MEWO_FOLDER_NAME)) {
    std::println(
        "error: failed to create {1} folder at \"{0}/{1}\"", path_str, Project::MEWO_FOLDER_NAME);
    return;
  }

  if (std::ofstream shader_file(absolute_path / Project::WGSL_SHADER_FILE_NAME);
      !shader_file || !shader_file.is_open()) {
    std::println(
        "error: failed to access \"{}/{}\" for writing", path_str, Project::WGSL_SHADER_FILE_NAME);
    return;
  } else {
    shader_file << code;
  }

  std::println("Saved as project \"{}\"", path_str);
}

}
