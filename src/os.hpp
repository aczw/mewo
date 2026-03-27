#pragma once

#include "window.hpp"

#include <imgui_impl_wgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <filesystem>

namespace mewo::os {

std::filesystem::path find_executable_dir();

ImGui_ImplWGPU_CreateSurfaceInfo retrieve_surface_info(
  const wgpu::Instance& instance,
  const Window& window
);

}  // namespace mewo::os
