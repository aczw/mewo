#include "impl/session/out_pass.hpp"

#include "core/gpu/string.hpp"
#include "impl/common/dimensions.hpp"

#include <webgpu/webgpu.h>

#include <string_view>
#include <utility>

namespace mewo::impl {
namespace {

constexpr WGPUTextureFormat kOutTexFormat = WGPUTextureFormat_RGBA8Unorm;

constexpr std::string_view kShaderSource = R"(
struct Uniforms {
  resolution: vec2f,
  time: f32,
  delta_time: f32,
};

@group(0) @binding(0) var<uniform> mw: Uniforms;

// Coordinates for drawing an oversized triangle (acts as a fullscreen quad)
const kUvCoords = array(
  vec2f(0.0, 0.0), vec2f(2.0, 0.0), vec2f(0.0, 2.0)
);

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> @builtin(position) vec4f {
  let uv = kUvCoords[index];

  // Derive clip space positions from UV coords. This gives us
  // [0] = (-1.0, -1.0), [1] = (3.0, -1.0), [2] = (-1.0, 3.0)
  let position_xy = (2.0 * uv) - 1.0;

  return vec4f(position_xy, 0.0, 1.0);
}

@fragment
fn fs_main(@builtin(position) position: vec4f) -> @location(0) vec4f {
  let uv = position.xy / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}
)";

}  // namespace

gpu::TexView create_out_tex(const gpu::Device& device, Dimensions render_size) {
  WGPUTextureDescriptor tex_desc = WGPU_TEXTURE_DESCRIPTOR_INIT;
  tex_desc.label = gpu::to_string_view("session-out-tex");
  tex_desc.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
  tex_desc.format = kOutTexFormat;
  tex_desc.size.width = render_size.width;
  tex_desc.size.height = render_size.height;

  gpu::Tex tex(wgpuDeviceCreateTexture(device.get(), &tex_desc));

  WGPUTextureViewDescriptor view_desc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
  view_desc.label = gpu::to_string_view("session-out-view");

  return gpu::TexView(wgpuTextureCreateView(tex.get(), &view_desc));
}

OutPass create_out_pass(const gpu::Device& device, Dimensions render_size) {
  WGPUBufferDescriptor unif_buf_desc = WGPU_BUFFER_DESCRIPTOR_INIT;
  unif_buf_desc.label = gpu::to_string_view("session-out-unif-buf");
  unif_buf_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
  unif_buf_desc.size = sizeof(OutPass::Uniforms);

  gpu::Buf unif_buf(wgpuDeviceCreateBuffer(device.get(), &unif_buf_desc));

  WGPUBindGroupLayoutEntry unif_bgl_entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
  unif_bgl_entry.binding = 0;
  unif_bgl_entry.visibility = WGPUShaderStage_Fragment;
  unif_bgl_entry.buffer.type = WGPUBufferBindingType_Uniform;
  unif_bgl_entry.buffer.minBindingSize = sizeof(OutPass::Uniforms);

  WGPUBindGroupLayoutDescriptor bgl_desc = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
  bgl_desc.label = gpu::to_string_view("session-out-bgl");
  bgl_desc.entryCount = 1;
  bgl_desc.entries = &unif_bgl_entry;

  gpu::BindGroupLayout bgl(wgpuDeviceCreateBindGroupLayout(device.get(), &bgl_desc));
  WGPUBindGroupLayout raw_bgl = bgl.get();

  WGPUBindGroupEntry unif_bg_entry = WGPU_BIND_GROUP_ENTRY_INIT;
  unif_bg_entry.binding = 0;
  unif_bg_entry.buffer = unif_buf.get();
  unif_bg_entry.size = sizeof(OutPass::Uniforms);

  WGPUBindGroupDescriptor bg_desc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
  bg_desc.label = gpu::to_string_view("session-out-bg");
  bg_desc.layout = bgl.get();
  bg_desc.entryCount = 1;
  bg_desc.entries = &unif_bg_entry;

  gpu::BindGroup bg(wgpuDeviceCreateBindGroup(device.get(), &bg_desc));

  WGPUPipelineLayoutDescriptor pl_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
  pl_desc.label = gpu::to_string_view("session-out-pl");
  pl_desc.bindGroupLayoutCount = 1;
  pl_desc.bindGroupLayouts = &raw_bgl;

  WGPUShaderSourceWGSL shader_source = WGPU_SHADER_SOURCE_WGSL_INIT;
  shader_source.code = gpu::to_string_view(kShaderSource);
  WGPUShaderModuleDescriptor shader_module_desc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
  shader_module_desc.nextInChain = &shader_source.chain;
  shader_module_desc.label = gpu::to_string_view("session-out-shader");

  gpu::ShaderModule shader_module(wgpuDeviceCreateShaderModule(device.get(), &shader_module_desc));

  gpu::PipelineLayout pl(wgpuDeviceCreatePipelineLayout(device.get(), &pl_desc));
  WGPURenderPipelineDescriptor rpl_desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
  rpl_desc.label = gpu::to_string_view("session-out-rpl");
  rpl_desc.layout = pl.get();
  rpl_desc.vertex.module = shader_module.get();
  rpl_desc.vertex.entryPoint = gpu::to_string_view("vs_main");

  WGPUColorTargetState color_target_state = WGPU_COLOR_TARGET_STATE_INIT;
  color_target_state.format = kOutTexFormat;
  WGPUFragmentState frag_state = WGPU_FRAGMENT_STATE_INIT;
  frag_state.module = shader_module.get();
  frag_state.entryPoint = gpu::to_string_view("fs_main");
  frag_state.targetCount = 1;
  frag_state.targets = &color_target_state;
  rpl_desc.fragment = &frag_state;

  gpu::RenderPipeline rpl(wgpuDeviceCreateRenderPipeline(device.get(), &rpl_desc));

  return {
    .unif_buf = std::move(unif_buf),
    .bg = std::move(bg),
    .rpl = std::move(rpl),
    .view = create_out_tex(device, render_size),
  };
}

}  // namespace mewo::impl
