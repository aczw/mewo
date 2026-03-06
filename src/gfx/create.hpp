#pragma once

#include "gfx/compilation_diagnostic.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace mewo::gfx::create {

using ShaderCompilationResult
    = std::pair<std::optional<wgpu::ShaderModule>, std::vector<CompilationDiagnostic>>;

ShaderCompilationResult shader_module_from_wgsl(
    const Renderer& renderer, std::string_view code, std::string_view label);

}
