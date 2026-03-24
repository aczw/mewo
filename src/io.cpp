#include "io.hpp"

#include "exception.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
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

}
