#pragma once

#include <filesystem>
#include <string_view>

namespace mewo {

class Assets {
  public:
  Assets();

  std::filesystem::path get(std::string_view relative_path) const;

  private:
  std::filesystem::path executable_path_;
  std::filesystem::path assets_path_;
};

}
