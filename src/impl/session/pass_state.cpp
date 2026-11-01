#include "impl/session/pass_state.hpp"

#include "core/gpu/string.hpp"

#include <webgpu/webgpu.h>

#include <format>
#include <string>
#include <string_view>

namespace mewo::impl {
namespace {

constexpr WGPUTextureFormat kPassTexFormat = WGPUTextureFormat_RGBA16Float;

}  // namespace

gpu::TexView create_tex(const gpu::Device& device, Dimensions render_size) {
  WGPUTextureDescriptor tex_desc = WGPU_TEXTURE_DESCRIPTOR_INIT;
  tex_desc.label = gpu::to_string_view("pass-tex");
  tex_desc.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
  tex_desc.format = kPassTexFormat;
  tex_desc.size.width = render_size.width;
  tex_desc.size.height = render_size.height;

  gpu::Tex tex(wgpuDeviceCreateTexture(device.get(), &tex_desc));

  WGPUTextureViewDescriptor view_desc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
  view_desc.label = gpu::to_string_view("pass-view");

  return gpu::TexView(wgpuTextureCreateView(tex.get(), &view_desc));
}

PassState::PassState(
  const gpu::Device& device,
  const gpu::BindGroupLayout& common_unifs_bgl,
  const Pass& pass,
  Dimensions render_size
)
    : id_(pass.id()), label_(std::format("render-pass (ID {})", pass.id())) {
  WGPUBindGroupLayout raw_common_unifs_bgl = common_unifs_bgl.get();

  WGPUPipelineLayoutDescriptor pipeline_layout_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
  std::string pipeline_layout_label = std::format("pass-pl (ID {})", pass.id());
  pipeline_layout_desc.label = gpu::to_string_view(pipeline_layout_label);
  pipeline_layout_desc.bindGroupLayoutCount = 1;
  pipeline_layout_desc.bindGroupLayouts = &raw_common_unifs_bgl;

  gpu::PipelineLayout pipeline_layout(
    wgpuDeviceCreatePipelineLayout(device.get(), &pipeline_layout_desc)
  );

  WGPUShaderSourceWGSL shader_source = WGPU_SHADER_SOURCE_WGSL_INIT;
  shader_source.code = gpu::to_string_view(pass.source());
  WGPUShaderModuleDescriptor shader_module_desc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
  shader_module_desc.nextInChain = &shader_source.chain;
  std::string shader_module_label = std::format("pass-shader (ID {})", pass.id());
  shader_module_desc.label = gpu::to_string_view(shader_module_label);

  gpu::ShaderModule shader_module(wgpuDeviceCreateShaderModule(device.get(), &shader_module_desc));

  WGPURenderPipelineDescriptor render_pipeline_desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
  std::string render_pipeline_label = std::format("pass-render-pipeline (ID {})", pass.id());
  render_pipeline_desc.label = gpu::to_string_view(render_pipeline_label);
  render_pipeline_desc.layout = pipeline_layout.get();
  render_pipeline_desc.vertex.module = shader_module.get();
  render_pipeline_desc.vertex.entryPoint = gpu::to_string_view("vs_main");

  WGPUColorTargetState color_target_state = WGPU_COLOR_TARGET_STATE_INIT;
  color_target_state.format = kPassTexFormat;
  WGPUFragmentState frag_state = WGPU_FRAGMENT_STATE_INIT;
  frag_state.module = shader_module.get();
  frag_state.entryPoint = gpu::to_string_view("fs_main");
  frag_state.targetCount = 1;
  frag_state.targets = &color_target_state;
  render_pipeline_desc.fragment = &frag_state;

  render_pipeline_ =
    gpu::RenderPipeline(wgpuDeviceCreateRenderPipeline(device.get(), &render_pipeline_desc));
  view_ = create_tex(device, render_size);
}

}  // namespace mewo::impl
