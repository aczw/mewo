#include "viewport.hpp"

#include "aspect_ratio.hpp"
#include "exception.hpp"
#include "gfx/create.hpp"
#include "gfx/renderer.hpp"
#include "gui/layout.hpp"
#include "query.hpp"

#include <SDL3/SDL_timer.h>
#include <webgpu/webgpu_cpp.h>

#include <cmath>
#include <filesystem>
#include <functional>
#include <optional>
#include <print>
#include <string_view>

namespace mewo {

Viewport::Viewport(const State& state, const gfx::Renderer& renderer, std::string_view initial_code)
{
  const wgpu::Device& device = renderer.device();
  const wgpu::SurfaceConfiguration& surface_config = renderer.surface_config();

  wgpu::BufferDescriptor unif_buf_desc = {
    .label = "viewport-uniform-buffer",
    .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
    .size = sizeof(Unif),
  };

  unif_buf_ = device.CreateBuffer(&unif_buf_desc);

  float width
      = std::floor(static_cast<float>(surface_config.width) * gui::Layout::SPLIT_LEFT_RATIO);
  float height = std::floor(width * AspectRatio::get_inverse_value(ratio_preset_));
  auto width_whole = static_cast<uint32_t>(width);
  auto height_whole = static_cast<uint32_t>(height);

  Unif unif = { .time = state.time, .resolution = { width, height } };
  renderer.queue().WriteBuffer(unif_buf_, 0, &unif, sizeof(Unif));

  wgpu::BindGroupLayoutEntry render_pipeline_unif_bgl_entry = {
    .binding = 0,
    .visibility = wgpu::ShaderStage::Fragment,
    .buffer = {
      .type = wgpu::BufferBindingType::Uniform,
      .minBindingSize = sizeof(Unif),
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
    .size = sizeof(Unif),
  };
  wgpu::BindGroupDescriptor render_pipeline_bg_desc = {
    .label = "viewport-render-pipeline-bind-group",
    .layout = render_pipeline_bgl_,
    .entryCount = 1,
    .entries = &render_pipeline_unif_bg_entry,
  };

  render_pipeline_bg_ = device.CreateBindGroup(&render_pipeline_bg_desc);

  color_target_state_ = { .format = surface_config.format };

  auto vert_shader_file_path = std::invoke([] -> std::filesystem::path {
    if constexpr (query::is_debug()) {
      return "../../assets/shaders/viewport.vert.wgsl";
    } else {
      throw Exception("TODO: handle \"viewport.vert.wgsl\" file path on release mode");
    }
  });

  wgpu::PipelineLayoutDescriptor render_pipeline_layout_desc = {
    .label = "viewport-render-pipeline-layout",
    .bindGroupLayoutCount = 1,
    .bindGroupLayouts = &render_pipeline_bgl_,
  };

  render_pipeline_desc_ = {
    .label = "viewport-render-pipeline",
    .layout = device.CreatePipelineLayout(&render_pipeline_layout_desc),
    .vertex = { .module = gfx::create::shader_module_from_wgsl(
                    device, vert_shader_file_path, "viewport-vert-shader-module"),
        .entryPoint = "main", },
  };

  set_fragment_state(device, initial_code);
  update_render_pipeline(device);

  texture_desc_ = {
    .label = "viewport-texture",
    .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
    .format = surface_config.format,
  };

  set_pending_resize(width_whole, height_whole);

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

const wgpu::TextureView& Viewport::view() const { return view_; }

Viewport::Mode Viewport::mode() const { return mode_; }

AspectRatio::Preset Viewport::ratio_preset() const { return ratio_preset_; }

uint32_t Viewport::width() const { return width_; }

uint32_t Viewport::height() const { return height_; }

void Viewport::set_fragment_state(const wgpu::Device& device, std::string_view code)
{
  fragment_state_ = {
    .module = gfx::create::shader_module_from_wgsl(device, code, "viewport-frag-shader-module"),
    .entryPoint = "main",
    .targetCount = 1,
    .targets = &color_target_state_,
  };
}

void Viewport::set_mode(Mode mode) { mode_ = mode; }

void Viewport::set_ratio_preset(AspectRatio::Preset preset) { ratio_preset_ = preset; }

void Viewport::set_width(uint32_t width) { width_ = width; };

void Viewport::set_height(uint32_t height) { height_ = height; };

void Viewport::set_pending_resize() { set_pending_resize(width_, height_); }

void Viewport::set_pending_resize(uint32_t new_width)
{
  float inverse_ratio = AspectRatio::get_inverse_value(ratio_preset_);
  float height = std::floor(static_cast<float>(new_width) * inverse_ratio);
  pending_resize_ = { new_width, static_cast<uint32_t>(height) };
}

void Viewport::set_pending_resize(uint32_t new_width, uint32_t new_height)
{
  pending_resize_ = { new_width, new_height };
}

void Viewport::record(const gfx::FrameContext& frame_ctx) const
{
  wgpu::RenderPassEncoder render_pass = frame_ctx.encoder.BeginRenderPass(&pass_desc_);

  render_pass.SetPipeline(render_pipeline_);
  render_pass.SetBindGroup(0, render_pipeline_bg_);
  render_pass.Draw(6);

  render_pass.End();
}

void Viewport::update_render_pipeline(const wgpu::Device& device)
{
  render_pipeline_desc_.fragment = &fragment_state_;
  render_pipeline_ = device.CreateRenderPipeline(&render_pipeline_desc_);
}

void Viewport::prepare_new_frame(State& state, const wgpu::Device& device, const wgpu::Queue& queue)
{
  if (pending_resize_.has_value()) {
    auto [new_width, new_height] = pending_resize_.value();

    if constexpr (query::is_debug())
      std::println("Viewport texture resized to {}×{}", new_width, new_height);

    // TODO: on initialization a couple of intermediary resizes occur, including
    //       a strange one to a resolution of 16×9 (yes, 16 pixels by 9 pixels)
    texture_desc_.size.width = new_width;
    texture_desc_.size.height = new_height;
    texture_ = device.CreateTexture(&texture_desc_);

    static const wgpu::TextureViewDescriptor VIEW_DESC = { .label = "viewport-view" };

    view_ = texture_.CreateView(&VIEW_DESC);
    pass_color_attachment_.view = view_;

    pending_resize_ = std::nullopt;
  }

  Unif unif = {
    .time = static_cast<float>(SDL_GetTicksNS()) / 1'000'000'000.f,
    .resolution
    = { static_cast<float>(texture_.GetWidth()), static_cast<float>(texture_.GetHeight()) },
  };

  queue.WriteBuffer(unif_buf_, 0, &unif, sizeof(Unif));

  // TODO: update time in the main render loop, not within this class
  state.time = unif.time;
}

}
