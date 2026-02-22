#include "create.hpp"

#include <string_view>

namespace mewo::gfx::create {

wgpu::ShaderModule shader_module_from_wgsl(
    const wgpu::Device& device, std::string_view code, std::string_view label)
{
  wgpu::ShaderSourceWGSL shader_source_wgsl = { { .code = code } };

  wgpu::ShaderModuleDescriptor shader_module_desc = {
    .nextInChain = &shader_source_wgsl,
    .label = label,
  };

  return device.CreateShaderModule(&shader_module_desc);
}

}
