#pragma once

#include "gfx/compilation_diagnostic.hpp"
#include "gfx/gfx.hpp"

#include <webgpu/webgpu_cpp.h>

#include <chrono>
#include <optional>
#include <string_view>

namespace mewo::gfx::create {

struct ShaderCompilationResult {
  std::optional<wgpu::ShaderModule> shader_module;
  gfx::CompilationDiagnostics diagnostics;
  std::chrono::duration<double> time_elapsed;
};

ShaderCompilationResult shader_module_from_wgsl(
  const Gfx& gfx,
  std::string_view code,
  std::string_view label
);

}  // namespace mewo::gfx::create
