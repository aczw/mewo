#include "impl/session.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"

#include <webgpu/webgpu.h>

#include <cstdint>
#include <memory>
#include <optional>
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

float ns_to_s(uint64_t time) { return static_cast<float>(static_cast<double>(time) * 1e-9); }

gpu::TexView create_out_tex(const gpu::Device& device, Dimensions render_size) {
  WGPUTextureDescriptor out_tex_desc = WGPU_TEXTURE_DESCRIPTOR_INIT;
  out_tex_desc.label = gpu::to_string_view("session-out-tex");
  out_tex_desc.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
  out_tex_desc.format = kOutTexFormat;
  out_tex_desc.size.width = render_size.width;
  out_tex_desc.size.height = render_size.height;

  gpu::Tex out_tex(wgpuDeviceCreateTexture(device.get(), &out_tex_desc));

  WGPUTextureViewDescriptor out_view_desc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
  out_view_desc.label = gpu::to_string_view("session-out-view");

  return gpu::TexView(wgpuTextureCreateView(out_tex.get(), &out_view_desc));
}

}  // namespace

std::optional<std::unique_ptr<Session>> Session::create(
  impl::Runtime& runtime,
  impl::Project& project,
  Dimensions render_size
) {
  const gpu::Device& device = runtime.device();

  // Device is not ready yet, cannot create a session
  if (!device)
    return std::nullopt;

  WGPUBufferDescriptor out_unif_buf_desc = WGPU_BUFFER_DESCRIPTOR_INIT;
  out_unif_buf_desc.label = gpu::to_string_view("session-out-unif-buf");
  out_unif_buf_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
  out_unif_buf_desc.size = sizeof(Uniforms);

  gpu::Buf out_unif_buf(wgpuDeviceCreateBuffer(device.get(), &out_unif_buf_desc));

  WGPUBindGroupLayoutEntry out_unif_bgl_entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
  out_unif_bgl_entry.binding = 0;
  out_unif_bgl_entry.visibility = WGPUShaderStage_Fragment;
  out_unif_bgl_entry.buffer.type = WGPUBufferBindingType_Uniform;
  out_unif_bgl_entry.buffer.minBindingSize = sizeof(Uniforms);

  WGPUBindGroupLayoutDescriptor out_bgl_desc = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
  out_bgl_desc.label = gpu::to_string_view("session-out-bgl");
  out_bgl_desc.entryCount = 1;
  out_bgl_desc.entries = &out_unif_bgl_entry;

  gpu::BindGroupLayout out_bgl(wgpuDeviceCreateBindGroupLayout(device.get(), &out_bgl_desc));
  WGPUBindGroupLayout raw_out_bgl = out_bgl.get();

  WGPUBindGroupEntry out_unif_bg_entry = WGPU_BIND_GROUP_ENTRY_INIT;
  out_unif_bg_entry.binding = 0;
  out_unif_bg_entry.buffer = out_unif_buf.get();
  out_unif_bg_entry.size = sizeof(Uniforms);

  WGPUBindGroupDescriptor out_bg_desc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
  out_bg_desc.label = gpu::to_string_view("session-out-bg");
  out_bg_desc.layout = out_bgl.get();
  out_bg_desc.entryCount = 1;
  out_bg_desc.entries = &out_unif_bg_entry;

  gpu::BindGroup out_bg(wgpuDeviceCreateBindGroup(device.get(), &out_bg_desc));

  WGPUPipelineLayoutDescriptor out_pl_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
  out_pl_desc.label = gpu::to_string_view("session-out-pl");
  out_pl_desc.bindGroupLayoutCount = 1;
  out_pl_desc.bindGroupLayouts = &raw_out_bgl;

  WGPUShaderSourceWGSL out_shader_source = WGPU_SHADER_SOURCE_WGSL_INIT;
  out_shader_source.code = gpu::to_string_view(kShaderSource);
  WGPUShaderModuleDescriptor out_shader_module_desc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
  out_shader_module_desc.nextInChain = &out_shader_source.chain;
  out_shader_module_desc.label = gpu::to_string_view("session-out-shader");

  gpu::ShaderModule out_shader_module(
    wgpuDeviceCreateShaderModule(device.get(), &out_shader_module_desc)
  );

  gpu::PipelineLayout out_pl(wgpuDeviceCreatePipelineLayout(device.get(), &out_pl_desc));
  WGPURenderPipelineDescriptor out_rpl_desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
  out_rpl_desc.label = gpu::to_string_view("session-out-rpl");
  out_rpl_desc.layout = out_pl.get();
  out_rpl_desc.vertex.module = out_shader_module.get();
  out_rpl_desc.vertex.entryPoint = gpu::to_string_view("vs_main");

  WGPUColorTargetState out_color_target_state = WGPU_COLOR_TARGET_STATE_INIT;
  out_color_target_state.format = kOutTexFormat;
  WGPUFragmentState out_frag_state = WGPU_FRAGMENT_STATE_INIT;
  out_frag_state.module = out_shader_module.get();
  out_frag_state.entryPoint = gpu::to_string_view("fs_main");
  out_frag_state.targetCount = 1;
  out_frag_state.targets = &out_color_target_state;
  out_rpl_desc.fragment = &out_frag_state;

  gpu::RenderPipeline out_rpl(wgpuDeviceCreateRenderPipeline(device.get(), &out_rpl_desc));

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Session>(new Session(
    runtime,
    project,
    render_size,
    std::move(out_unif_buf),
    std::move(out_bg),
    std::move(out_rpl),
    create_out_tex(device, render_size)
  ));
}

void Session::set_render_size(Dimensions render_size) {
  if (render_size_.width == render_size.width && render_size_.height == render_size.height)
    return;

  render_size_ = render_size;
  out_view_ = create_out_tex(runtime_.device(), render_size);
}

Session::Session(
  impl::Runtime& runtime,
  impl::Project& project,
  Dimensions render_size,
  gpu::Buf&& out_unif_buf,
  gpu::BindGroup&& out_bg,
  gpu::RenderPipeline&& out_rpl,
  gpu::TexView&& out_view
)
    : runtime_(runtime),
      project_(project),
      render_size_(render_size),
      previous_time_(std::nullopt),
      elapsed_(0),
      out_unif_buf_(std::move(out_unif_buf)),
      out_bg_(std::move(out_bg)),
      out_rpl_(std::move(out_rpl)),
      out_view_(std::move(out_view)) {}

void Session::render(impl::Surface* surface, uint64_t current_time) {
  const gpu::Queue& queue = runtime_.queue();
  uint64_t delta_time = 0;

  if (previous_time_ && current_time > *previous_time_)
    delta_time = current_time - *previous_time_;

  previous_time_ = current_time;
  elapsed_ += delta_time;

  Uniforms out_unif = {
    .resolution =
      {
        static_cast<float>(render_size_.width),
        static_cast<float>(render_size_.height),
      },
    .time = ns_to_s(elapsed_),
    .delta_time = ns_to_s(delta_time),
  };
  wgpuQueueWriteBuffer(queue.get(), out_unif_buf_.get(), 0, &out_unif, sizeof(Uniforms));

  WGPUCommandEncoderDescriptor cmd_encoder_desc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
  cmd_encoder_desc.label = gpu::to_string_view("session-cmd-encoder");

  gpu::CmdEncoder cmd_encoder(
    wgpuDeviceCreateCommandEncoder(runtime_.device().get(), &cmd_encoder_desc)
  );

  WGPURenderPassColorAttachment out_pass_color_attachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
  out_pass_color_attachment.view = out_view_.get();
  out_pass_color_attachment.loadOp = WGPULoadOp_Clear;
  out_pass_color_attachment.storeOp = WGPUStoreOp_Store;
  WGPURenderPassDescriptor out_pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
  out_pass_desc.label = gpu::to_string_view("session-out-pass");
  out_pass_desc.colorAttachmentCount = 1;
  out_pass_desc.colorAttachments = &out_pass_color_attachment;

  gpu::RenderPassEncoder out_encoder(
    wgpuCommandEncoderBeginRenderPass(cmd_encoder.get(), &out_pass_desc)
  );

  wgpuRenderPassEncoderSetPipeline(out_encoder.get(), out_rpl_.get());
  wgpuRenderPassEncoderSetBindGroup(out_encoder.get(), 0, out_bg_.get(), 0, nullptr);
  wgpuRenderPassEncoderDraw(out_encoder.get(), 3, 0, 0, 0);
  wgpuRenderPassEncoderEnd(out_encoder.get());

  WGPUCommandBufferDescriptor cmd_buf_desc = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
  cmd_buf_desc.label = gpu::to_string_view("session-cmd-buf");

  gpu::CmdBuf cmd_buf(wgpuCommandEncoderFinish(cmd_encoder.get(), &cmd_buf_desc));
  WGPUCommandBuffer raw_cmd_buf = cmd_buf.get();
  wgpuQueueSubmit(queue.get(), 1, &raw_cmd_buf);
}

}  // namespace mewo::impl
