#include "os.hpp"

#include "exception.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_platform_defines.h>
#include <SDL3/SDL_video.h>

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
#error MEWO_UNSUPPORTED_PLATFORM_MSG
  throw Exception(MEWO_UNSUPPORTED_PLATFORM_MSG);
#endif
}

ImGui_ImplWGPU_CreateSurfaceInfo retrieve_surface_info(
  const wgpu::Instance& instance,
  const Window& window
) {
  SDL_PropertiesID window_props_id = SDL_GetWindowProperties(window.get());

  if (window_props_id == 0)
    throw Exception("Failed to get SDL window properties: {}", SDL_GetError());

  return {
    .Instance = instance.Get(),
#if defined(SDL_PLATFORM_MACOS)
    .System = "cocoa",
    .RawWindow = static_cast<void*>(
      SDL_GetPointerProperty(window_props_id, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr)
    ),
#elif defined(SDL_PLATFORM_WIN32)
    .System = "win32",
    .RawWindow = static_cast<void*>(
      SDL_GetPointerProperty(window_props_id, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)
    ),
    .RawInstance = static_cast<void*>(GetModuleHandle(nullptr)),
#else
#error MEWO_UNSUPPORTED_PLATFORM_MSG
#endif
  };
}

}  // namespace mewo::os
