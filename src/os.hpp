#pragma once

#include "window.hpp"

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_platform_defines.h>
#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace mewo::os {

/// Use the Command key on macOS and Ctrl on Windows.
#if defined(SDL_PLATFORM_MACOS)
inline constexpr uint16_t PRIMARY_MOD_KEY = SDL_KMOD_GUI;
inline constexpr std::string_view PRIMARY_MOD_LABEL = "⌘";
#elif defined(SDL_PLATFORM_WINDOWS)
inline constexpr uint16_t PRIMARY_MOD_KEY = SDL_KMOD_CTRL;
inline constexpr std::string_view PRIMARY_MOD_LABEL = "Ctrl";
#else
#error MEWO_UNSUPPORTED_PLATFORM_MSG
#endif

std::filesystem::path find_executable_dir();

ImGui_ImplWGPU_CreateSurfaceInfo retrieve_surface_info(
  const wgpu::Instance& instance,
  const Window& window
);

}  // namespace mewo::os
