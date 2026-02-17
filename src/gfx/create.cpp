#include "create.hpp"

#include "fs.hpp"

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

wgpu::ShaderModule shader_module_from_wgsl(
    const wgpu::Device& device, const std::filesystem::path& file_path, std::string_view label)
{
  std::string code = fs::read_wgsl_shader(file_path);

  // Explicitly conversion to resolve function overload
  return shader_module_from_wgsl(device, std::string_view(code), label);
}

}
