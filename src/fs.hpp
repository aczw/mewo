#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace mewo::fs {

std::string read_file(const std::filesystem::path& file_path);

/// Reads a plain text WGSL shader from disk.
std::string read_wgsl_shader(const std::filesystem::path& file_path);

/// Saves the current state of Mewo as a new project.
void save_as(const std::filesystem::path& folder_path, std::string_view code);

}
