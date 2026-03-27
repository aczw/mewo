#include "assets.hpp"

#include "exception.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <filesystem>
#include <print>

namespace mewo {

Assets::Assets(const std::filesystem::path& executable_dir) {
  using namespace std::string_view_literals;

  // There are two possible places where assets can be located, depending on whether
  // the executable is launched from the build directory (during development), or from
  // the final release folder. This way I can support both.
  static constexpr std::array POSSIBLE_ASSETS_DIRS = {
    "./assets"sv,
    "../../assets"sv,
  };

  for (auto asset_dir : POSSIBLE_ASSETS_DIRS) {
    if (auto full_path = executable_dir / asset_dir; std::filesystem::exists(full_path)) {
      directory_ = std::filesystem::canonical(full_path);
      break;
    }
  }

  if (directory_.empty()) {
    throw Exception("Assets directory was not found");
  } else {
    std::println("Assets directory found at \"{}\"", directory_.string());
  }
}

}  // namespace mewo
