#include "os.hpp"

#include "exception.hpp"

#include <SDL3/SDL_platform_defines.h>

#if defined(SDL_PLATFORM_MACOS)
#include <mach-o/dyld.h>
#include <vector>
#elif defined(SDL_PLATFORM_WIN32)
#include <array>
#include <windows.h>
#endif

namespace mewo::os {

std::filesystem::path find_executable_dir() {
  static constexpr size_t MAX_FILE_PATH_LENGTH = 1024;

#if defined(SDL_PLATFORM_MACOS)
  uint32_t buf_size = MAX_FILE_PATH_LENGTH;
  std::vector<char> buf(buf_size);

  if (_NSGetExecutablePath(buf.data(), &buf_size) == -1) {
    buf.resize(buf_size);

    // Resize and try again. If it fails again, then we're in big trouble
    if (_NSGetExecutablePath(buf.data(), &buf_size) == -1)
      throw Exception("Call to _NSGetExecutablePath failed: buffer size not large enough");
  }

  // Path returned by `_NSGetExecutablePath` needs to be resolved
  return std::filesystem::canonical(buf.data()).parent_path();
#elif defined(SDL_PLATFORM_WIN32)
  std::array<wchar_t, MAX_FILE_PATH_LENGTH> buf = {};

  if (GetModuleFileNameW(nullptr, buf.data(), MAX_FILE_PATH_LENGTH) == 0)
    throw Exception("Call to GetModuleFileNameW failed");

  return std::filesystem::path(buf.data()).parent_path();
#else
#error "Unsupported platform. Supported platforms are macOS and Windows"
  throw Exception("Unsupported platform. Supported platforms are macOS and Windows");
#endif
}

}  // namespace mewo::os
