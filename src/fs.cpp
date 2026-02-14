#include "fs.hpp"

#include "exception.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <string_view>

namespace mewo::fs {

static constexpr std::string_view WGSL_FILE_EXTENSION = ".wgsl";

std::string read_wgsl_shader(const std::filesystem::path& file_path)
{
  namespace std_fs = std::filesystem;

  // File path may not exist so it can only be converted weakly
  auto path_str = std_fs::weakly_canonical(file_path).string();

  if (!std_fs::exists(file_path) || !std_fs::is_regular_file(file_path))
    throw Exception("File \"{}\" does not exist or is not regular", path_str);

  if (file_path.extension() != WGSL_FILE_EXTENSION)
    throw Exception(
        "\"{}\" is not a WGSL shader (does not end with {})", path_str, WGSL_FILE_EXTENSION);

  // Open in binary mode because otherwise it might do weird things with newlines/carriage returns
  std::ifstream file(file_path, std::ios::in | std::ios::binary);

  if (!file || !file.is_open())
    throw Exception("Failed to open WGSL shader at \"{}\"", path_str);

  auto file_size = static_cast<size_t>(std_fs::file_size(file_path));

  std::string source;
  source.resize(file_size);

  file.seekg(0, std::ios::beg);
  file.read(source.data(), static_cast<std::streamsize>(file_size));

  return source;
}

}
