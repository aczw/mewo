#pragma once

#include "gfx/compilation_diagnostic.hpp"
#include "gfx/gfx.hpp"

#include <webgpu/webgpu_cpp.h>

#include <optional>
#include <string_view>
#include <utility>

namespace mewo::gfx::create {

using ShaderCompilationResult =
  std::pair<std::optional<wgpu::ShaderModule>, gfx::CompilationDiagnostics>;

ShaderCompilationResult shader_module_from_wgsl(
  const Gfx& gfx,
  std::string_view code,
  std::string_view label
);

}  // namespace mewo::gfx::create
