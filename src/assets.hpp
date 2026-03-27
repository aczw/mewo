#pragma once

#include <filesystem>
#include <string_view>

namespace mewo {

class Assets {
 public:
  Assets();

  std::filesystem::path get(std::string_view relative_path) const {
    return assets_directory_ / relative_path;
  }

 private:
  std::filesystem::path executable_directory_;
  std::filesystem::path assets_directory_;
};

}  // namespace mewo
