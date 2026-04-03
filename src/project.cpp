#include "project.hpp"

#include "exception.hpp"
#include "query.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <print>

namespace mewo {

Project::Project(const std::filesystem::path& folder_to_open) {
  namespace fs = std::filesystem;

  auto path_str = fs::weakly_canonical(folder_to_open).string();

  if (!fs::exists(folder_to_open))
    throw Exception("Project folder \"{}\" does not exist", path_str);

  auto absolute = fs::absolute(folder_to_open);
  path_str = absolute.string();

  if (!fs::is_directory(absolute))
    throw Exception("\"{}\" is not a folder", path_str);

  auto mewo_dir = absolute / MEWO_FOLDER_NAME;
  if (!fs::exists(mewo_dir) || !fs::is_directory(mewo_dir))
    throw Exception("\"{}\" either does not exist or is not a folder", mewo_dir.string());

  std::ifstream project_json_file(mewo_dir / PROJECT_JSON_FILE_NAME);

  if (!project_json_file || !project_json_file.is_open())
    throw Exception("Failed to open \"{}/{}\"", mewo_dir.string(), PROJECT_JSON_FILE_NAME);

  using json = nlohmann::json;
  json data = json::parse(project_json_file);

  if (
    auto schema_version = data[SCHEMA_VERSION_KEY].get<decltype(SCHEMA_VERSION_VALUE)>();
    schema_version != SCHEMA_VERSION_VALUE
  ) {
    throw Exception(
      "Unexpected schema version found: {} (found) != {} (expected)",
      schema_version,
      SCHEMA_VERSION_VALUE
    );
  } else {
    std::println("Schema version: {}", schema_version);
  }

  root_directory_ = absolute;
  name_ = root_directory_.filename().string();
}

Project Project::save_as(const std::filesystem::path& directory, std::string_view code) {
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

  auto mewo_dir = absolute_path / MEWO_FOLDER_NAME;
  if (!fs::create_directory(mewo_dir))
    throw Exception("Failed to create {1} folder at \"{0}/{1}\"", path_str, MEWO_FOLDER_NAME);

  auto new_project = Project(absolute_path, SkipPathValidationTag{});
  new_project.save(code);

  if (
    std::ofstream project_json_file(mewo_dir / PROJECT_JSON_FILE_NAME);
    !project_json_file || !project_json_file.is_open()
  ) {
    throw Exception(
      "Failed to access {}/{} for writing", mewo_dir.string(), PROJECT_JSON_FILE_NAME
    );
  } else {
    namespace ch = std::chrono;
    auto now = ch::system_clock::now();
    int64_t created_at_timestamp = ch::duration_cast<ch::seconds>(now.time_since_epoch()).count();

    nlohmann::ordered_json data = {
      {SCHEMA_VERSION_KEY, SCHEMA_VERSION_VALUE},
      {"created_at", created_at_timestamp},
    };

    project_json_file << data.dump(4);
  }

  if constexpr (query::is_debug())
    std::println("Saved as project \"{}\"", path_str);

  return new_project;
}

void Project::save(std::string_view code) const {
  auto file_location = shader_file_location();

  if (std::ofstream shader_file(file_location); !shader_file || !shader_file.is_open()) {
    throw Exception("Failed to access \"{}\" for writing", file_location.string());
  } else {
    shader_file << code;
  }
}

Project::Project(const std::filesystem::path& existing_directory, SkipPathValidationTag)
    : root_directory_(std::filesystem::absolute(existing_directory)),
      name_(root_directory_.filename().string()) {}

}  // namespace mewo
