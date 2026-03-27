#pragma once

#include <filesystem>
#include <string_view>

namespace mewo {

class Assets {
 public:
  explicit Assets(const std::filesystem::path& executable_dir);

  std::filesystem::path get(std::string_view relative_path) const {
    return directory_ / relative_path;
  }

 private:
  std::filesystem::path directory_;
};

}  // namespace mewo
