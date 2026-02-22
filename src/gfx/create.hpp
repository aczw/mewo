#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string_view>

namespace mewo::gfx::create {

wgpu::ShaderModule shader_module_from_wgsl(
    const wgpu::Device& device, std::string_view code, std::string_view label);

}
