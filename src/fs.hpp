#pragma once

#include <filesystem>
#include <string>

namespace mewo::fs {

std::string read_file(const std::filesystem::path& file_path);

/// Reads a plain text WGSL shader from disk.
std::string read_wgsl_shader(const std::filesystem::path& file_path);

}
