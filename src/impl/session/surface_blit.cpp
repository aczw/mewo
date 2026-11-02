#include "impl/session/surface_blit.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"

#include <webgpu/webgpu.h>

#include <string_view>

namespace mewo::impl {
namespace {

constexpr std::string_view kShaderSource = R"(
@group(0) @binding(0) var source_tex: texture_2d<f32>;
@group(0) @binding(1) var source_smp: sampler;

struct VertexOut {
  @builtin(position) position: vec4f,
  @location(0) uv: vec2f,
};

// Coordinates for drawing an oversized triangle (acts as a fullscreen quad)
const kUvCoords = array(
  vec2f(0.0, 0.0), vec2f(2.0, 0.0), vec2f(0.0, 2.0)
);

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> VertexOut {
  let uv = kUvCoords[index];

  var out: VertexOut;
  // Derive clip space positions from UV coords. This gives us
  // [0] = (-1.0, -1.0), [1] = (3.0, -1.0), [2] = (-1.0, 3.0)
  out.position = vec4f((2.0 * uv) - 1.0, 0.0, 1.0);
  out.uv = uv;
  return out;
}

@fragment
fn fs_main(in: VertexOut) -> @location(0) vec4f {
  return textureSample(source_tex, source_smp, in.uv);
}
)";

}  // namespace

void SurfaceBlit::prepare(
  const gpu::Device& device,
  const gpu::BindGroupLayout& bind_group_layout,
  WGPUTextureFormat surface_format
) {
  if (render_pipeline_ && current_surface_format_ == surface_format)
    return;

  WGPUShaderSourceWGSL shader_source = WGPU_SHADER_SOURCE_WGSL_INIT;
  shader_source.code = gpu::to_string_view(kShaderSource);
  WGPUShaderModuleDescriptor shader_module_desc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
  shader_module_desc.nextInChain = &shader_source.chain;
  shader_module_desc.label = gpu::to_string_view("surface-blit-shader");

  gpu::ShaderModule shader_module(wgpuDeviceCreateShaderModule(device.get(), &shader_module_desc));

  WGPUBindGroupLayout raw_bgl = bind_group_layout.get();
  WGPUPipelineLayoutDescriptor pl_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
  pl_desc.label = gpu::to_string_view("surface-blit-pipeline-layout");
  pl_desc.bindGroupLayoutCount = 1;
  pl_desc.bindGroupLayouts = &raw_bgl;

  gpu::PipelineLayout pl(wgpuDeviceCreatePipelineLayout(device.get(), &pl_desc));

  WGPURenderPipelineDescriptor rpl_desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
  rpl_desc.label = gpu::to_string_view("surface-blit-render-pipeline");
  rpl_desc.layout = pl.get();
  rpl_desc.vertex.module = shader_module.get();
  rpl_desc.vertex.entryPoint = gpu::to_string_view("vs_main");

  WGPUColorTargetState color_target_state = WGPU_COLOR_TARGET_STATE_INIT;
  color_target_state.format = surface_format;
  WGPUFragmentState frag_state = WGPU_FRAGMENT_STATE_INIT;
  frag_state.module = shader_module.get();
  frag_state.entryPoint = gpu::to_string_view("fs_main");
  frag_state.targetCount = 1;
  frag_state.targets = &color_target_state;
  rpl_desc.fragment = &frag_state;

  render_pipeline_ = gpu::RenderPipeline(wgpuDeviceCreateRenderPipeline(device.get(), &rpl_desc));
  current_surface_format_ = surface_format;
}

}  // namespace mewo::impl
