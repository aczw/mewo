#include "project.hpp"

#include "exception.hpp"

#include <filesystem>

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

  root_ = absolute;
  name_ = root_.filename().string();
}

const std::filesystem::path& Project::root() const { return root_; }

}
