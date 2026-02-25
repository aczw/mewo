#include "assets.hpp"

#include "exception.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <filesystem>
#include <print>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#endif

namespace mewo {

static std::filesystem::path get_executable_path()
{
  static constexpr size_t MAX_FILE_PATH_LENGTH = 1024;

#if defined(SDL_PLATFORM_MACOS)
#error "TODO: find executable path on macOS"
  throw Exception("TODO: find executable path on macOS");
#elif defined(SDL_PLATFORM_WIN32)
  std::array<wchar_t, MAX_FILE_PATH_LENGTH> path_arr = {};

  if (GetModuleFileNameW(nullptr, path_arr.data(), MAX_FILE_PATH_LENGTH) == 0)
    throw Exception("Call to GetModuleFileNameW failed");

  return std::filesystem::path(path_arr.data()).parent_path();
#else
#error "Unsupported platform. Supported platforms are macOS and Windows"
  throw Exception("Unsupported platform. Supported platforms are macOS and Windows");
#endif
}

Assets::Assets()
    : executable_path_(get_executable_path())
{
  using namespace std::string_view_literals;

  // There are two possible places where assets can be located, depending on whether
  // the executable is launched from the build directory (during development), or from
  // the final release folder. This way I can support both.
  static constexpr std::array POSSIBLE_ASSETS_DIRS = {
    "./assets"sv,
    "../../assets"sv,
  };

  for (auto asset_dir : POSSIBLE_ASSETS_DIRS) {
    if (auto full_path = executable_path_ / asset_dir; std::filesystem::exists(full_path)) {
      assets_path_ = std::filesystem::canonical(full_path);
      break;
    }
  }

  if (assets_path_.empty()) {
    throw Exception("Assets directory was not found");
  } else {
    std::println("Assets directory found at \"{}\"", assets_path_.string());
  }
}

std::filesystem::path Assets::get(std::string_view relative_path) const
{
  return assets_path_ / relative_path;
}

}
