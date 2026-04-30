#include "viewport.hpp"

#include "aspect_ratio.hpp"
#include "event/event.hpp"
#include "event/event_queue.hpp"
#include "exception.hpp"
#include "gfx/create.hpp"
#include "gfx/gfx.hpp"
#include "gui/gui.hpp"
#include "io.hpp"

#include <imgui.h>
#include <webgpu/webgpu_cpp.h>

#include <cmath>
#include <cstdint>
#include <string_view>

namespace mewo {

Viewport::Viewport(
  EventQueue& event_queue,
  const std::filesystem::path& assets_dir,
  const gfx::Gfx& gfx,
  std::string_view initial_code
) {
  const wgpu::Device& device = gfx.device();
  const wgpu::SurfaceConfiguration& surface_config = gfx.surface_config();

  wgpu::BufferDescriptor unif_buf_desc = {
    .label = "viewport-uniform-buffer",
    .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
    .size = sizeof(Uniforms),
  };

  unif_buf_ = device.CreateBuffer(&unif_buf_desc);

  float width = std::floor(static_cast<float>(surface_config.width) * Gui::SPLIT_LEFT_RATIO);
  float height = std::floor(width * AspectRatio::get_inverse_value(ratio_preset_));
  auto width_whole = static_cast<uint32_t>(width);
  auto height_whole = static_cast<uint32_t>(height);

  Uniforms unif = {.resolution = {width, height}};
  gfx.queue().WriteBuffer(unif_buf_, 0, &unif, sizeof(Uniforms));

  wgpu::BindGroupLayoutEntry render_pipeline_unif_bgl_entry = {
    .binding = 0,
    .visibility = wgpu::ShaderStage::Fragment,
    .buffer = {
      .type = wgpu::BufferBindingType::Uniform,
      .minBindingSize = sizeof(Uniforms),
    },
  };

  wgpu::BindGroupLayoutDescriptor render_pipeline_bgl_desc = {
    .label = "viewport-render-pipeline-bind-group-layout",
    .entryCount = 1,
    .entries = &render_pipeline_unif_bgl_entry,
  };
  render_pipeline_bgl_ = device.CreateBindGroupLayout(&render_pipeline_bgl_desc);

  wgpu::BindGroupEntry render_pipeline_unif_bg_entry = {
    .binding = 0,
    .buffer = unif_buf_,
    .size = sizeof(Uniforms),
  };

  wgpu::BindGroupDescriptor render_pipeline_bg_desc = {
    .label = "viewport-render-pipeline-bind-group",
    .layout = render_pipeline_bgl_,
    .entryCount = 1,
    .entries = &render_pipeline_unif_bg_entry,
  };

  render_pipeline_bg_ = device.CreateBindGroup(&render_pipeline_bg_desc);

  color_target_state_ = {.format = surface_config.format};

  wgpu::PipelineLayoutDescriptor render_pipeline_layout_desc = {
    .label = "viewport-render-pipeline-layout",
    .bindGroupLayoutCount = 1,
    .bindGroupLayouts = &render_pipeline_bgl_,
  };

  const auto& [vert_module_opt, vert_diagnostics] = gfx::create::shader_module_from_wgsl(
    gfx, io::read_wgsl_shader(assets_dir / "shaders/viewport.vert.wgsl"), "viewport-vert-shader"
  );

  if (!vert_module_opt.has_value()) {
    throw Exception(
      "Compiling viewport vertex shader failed! {} diagnostics reported", vert_diagnostics.size()
    );
  }

  render_pipeline_desc_ = {
    .label = "viewport-render-pipeline",
    .layout = device.CreatePipelineLayout(&render_pipeline_layout_desc),
    .vertex = {.module = vert_module_opt.value(), .entryPoint = "main"},
  };

  // Compile a default fragment shader to enable render pipeline creation in constructor
  const auto& [frag_module, frag_diagnostics] = gfx::create::shader_module_from_wgsl(
    gfx, io::read_wgsl_shader(assets_dir / "shaders/viewport.frag.wgsl"), FRAGMENT_SHADER_LABEL
  );

  if (!frag_module.has_value()) {
    throw Exception(
      "Compiling default viewport fragment shader failed! {} diagnostics reported",
      frag_diagnostics.size()
    );
  }

  rebuild_render_pipeline(frag_module.value(), device);
  event_queue.push(RunRequest{.fragment_code = std::string(initial_code)});

  texture_desc_ = {
    .label = "viewport-texture",
    .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
    .format = surface_config.format,
  };

  event_queue.push(ViewportResizeRequest{.new_width = width_whole, .new_height = height_whole});

  // Use the width and height from the aspect ratio preset as initial values
  width_ = width_whole;
  height_ = height_whole;

  pass_color_attachment_ = {
    .view = view_,
    .loadOp = wgpu::LoadOp::Clear,
    .storeOp = wgpu::StoreOp::Store,
  };

  pass_desc_ = {
    .label = "viewport-render-pass",
    .colorAttachmentCount = 1,
    .colorAttachments = &pass_color_attachment_,
  };
}

void Viewport::update(
  const wgpu::Queue& queue,
  const gfx::FrameContext& frame_ctx,
  float current_time,
  float delta_time
) const {
  Uniforms unif = {
    .resolution =
      {static_cast<float>(texture_.GetWidth()), static_cast<float>(texture_.GetHeight())},
    .time = current_time,
    .delta_time = delta_time,
    .frame_number = frame_ctx.number,
    // TODO: Shadertoy counts how many frames were renderered in the last second instead. Use that?
    .frame_rate = static_cast<uint32_t>(ImGui::GetIO().Framerate),
  };

  queue.WriteBuffer(unif_buf_, 0, &unif, sizeof(unif));

  {
    wgpu::RenderPassEncoder render_pass = frame_ctx.encoder.BeginRenderPass(&pass_desc_);

    render_pass.SetPipeline(render_pipeline_);
    render_pass.SetBindGroup(0, render_pipeline_bg_);
    render_pass.Draw(6);

    render_pass.End();
  }

}  // namespace mewo

void Viewport::rebuild_render_pipeline(
  const wgpu::ShaderModule& fragment_module,
  const wgpu::Device& device
) {
  fragment_state_ = {
    .module = fragment_module,
    .entryPoint = "main",
    .targetCount = 1,
    .targets = &color_target_state_,
  };

  render_pipeline_desc_.fragment = &fragment_state_;
  render_pipeline_ = device.CreateRenderPipeline(&render_pipeline_desc_);
}

void Viewport::resize(const wgpu::Device& device, uint32_t new_width, uint32_t new_height) {
  // TODO: multiply by display scale to get actual pixel resolution
  texture_desc_.size.width = new_width;
  texture_desc_.size.height = new_height;
  texture_ = device.CreateTexture(&texture_desc_);

  static constexpr wgpu::TextureViewDescriptor VIEW_DESC = {.label = "viewport-view"};

  view_ = texture_.CreateView(&VIEW_DESC);
  pass_color_attachment_.view = view_;
}

}  // namespace mewo
