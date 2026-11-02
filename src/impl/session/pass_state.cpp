#include "impl/session/pass_state.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "core/shader/assembler.hpp"
#include "impl/session/pass_targets.hpp"

#include <webgpu/webgpu.h>

#include <array>
#include <format>
#include <string>
#include <string_view>

namespace mewo::impl {

PassState::PassState(
  const gpu::Device& device,
  const gpu::BindGroupLayout& common_unifs_bgl,
  const gpu::BindGroupLayout& prev_bgl,
  const Pass& pass
)
    : label_(std::format("render-pass (ID {})", pass.id())) {
  std::array<WGPUBindGroupLayout, 2> bgl_entries = {common_unifs_bgl.get(), prev_bgl.get()};

  WGPUPipelineLayoutDescriptor pipeline_layout_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
  std::string pipeline_layout_label = std::format("pass-pl (ID {})", pass.id());
  pipeline_layout_desc.label = gpu::to_string_view(pipeline_layout_label);
  pipeline_layout_desc.bindGroupLayoutCount = bgl_entries.size();
  pipeline_layout_desc.bindGroupLayouts = bgl_entries.data();

  gpu::PipelineLayout pipeline_layout(
    wgpuDeviceCreatePipelineLayout(device.get(), &pipeline_layout_desc)
  );

  WGPUShaderSourceWGSL shader_source = WGPU_SHADER_SOURCE_WGSL_INIT;
  std::string shader_source_code = assemble_pass_source(pass.source());
  shader_source.code = gpu::to_string_view(shader_source_code);
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
  render_pipeline_desc.vertex.entryPoint = gpu::to_string_view(kVertexEntryPoint);

  WGPUColorTargetState color_target_state = WGPU_COLOR_TARGET_STATE_INIT;
  color_target_state.format = PassTargets::kTexFormat;
  WGPUFragmentState frag_state = WGPU_FRAGMENT_STATE_INIT;
  frag_state.module = shader_module.get();
  frag_state.entryPoint = gpu::to_string_view(kFragmentEntryPoint);
  frag_state.targetCount = 1;
  frag_state.targets = &color_target_state;
  render_pipeline_desc.fragment = &frag_state;

  render_pipeline_ =
    gpu::RenderPipeline(wgpuDeviceCreateRenderPipeline(device.get(), &render_pipeline_desc));
}

}  // namespace mewo::impl
