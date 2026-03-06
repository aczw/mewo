#include "viewport.hpp"

#include "aspect_ratio.hpp"
#include "exception.hpp"
#include "fs.hpp"
#include "gfx/create.hpp"
#include "gfx/renderer.hpp"
#include "gui/layout.hpp"
#include "query.hpp"

#include <SDL3/SDL_timer.h>
#include <webgpu/webgpu_cpp.h>

#include <cmath>
#include <optional>
#include <print>
#include <string_view>
#include <utility>

namespace mewo {

static constexpr std::string_view DEFAULT_FRAG_SHADER_LABEL = "viewport-frag-shader";

Viewport::Viewport(const Assets& assets, const State& state, const gfx::Renderer& renderer,
    std::string_view initial_code)
{
  const wgpu::Device& device = renderer.device();
  const wgpu::SurfaceConfiguration& surface_config = renderer.surface_config();

  wgpu::BufferDescriptor unif_buf_desc = {
    .label = "viewport-uniform-buffer",
    .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
    .size = sizeof(Uniforms),
  };

  unif_buf_ = device.CreateBuffer(&unif_buf_desc);

  float width
      = std::floor(static_cast<float>(surface_config.width) * gui::Layout::SPLIT_LEFT_RATIO);
  float height = std::floor(width * AspectRatio::get_inverse_value(ratio_preset_));
  auto width_whole = static_cast<uint32_t>(width);
  auto height_whole = static_cast<uint32_t>(height);

  Uniforms unif = { .time = state.time, .resolution = { width, height } };
  renderer.queue().WriteBuffer(unif_buf_, 0, &unif, sizeof(Uniforms));

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

  color_target_state_ = { .format = surface_config.format };

  wgpu::PipelineLayoutDescriptor render_pipeline_layout_desc = {
    .label = "viewport-render-pipeline-layout",
    .bindGroupLayoutCount = 1,
    .bindGroupLayouts = &render_pipeline_bgl_,
  };

  const auto& [vert_module_opt, vert_diagnostics] = gfx::create::shader_module_from_wgsl(renderer,
      fs::read_wgsl_shader(assets.get("shaders/viewport.vert.wgsl")), "viewport-vert-shader");

  if (!vert_module_opt.has_value()) {
    throw Exception("Compiling viewport vertex shader failed! {} diagnostics reported",
        vert_diagnostics.size());
  }

  render_pipeline_desc_ = {
    .label = "viewport-render-pipeline",
    .layout = device.CreatePipelineLayout(&render_pipeline_layout_desc),
    .vertex = { .module = vert_module_opt.value(), .entryPoint = "main" },
  };

  // Compile a default fragment shader to enable render pipeline creation in constructor
  const auto& [frag_module_opt, frag_diagnostics] = gfx::create::shader_module_from_wgsl(renderer,
      fs::read_wgsl_shader(assets.get("shaders/viewport.frag.wgsl")),
      DEFAULT_FRAG_SHADER_LABEL.data());

  if (!frag_module_opt.has_value()) {
    throw Exception("Compiling default viewport fragment shader failed! {} diagnostics reported",
        frag_diagnostics.size());
  }

  fragment_state_ = {
    .module = frag_module_opt.value(),
    .entryPoint = "main",
    .targetCount = 1,
    .targets = &color_target_state_,
  };

  update_render_pipeline(device);

  // Also send off a compilation request for the actual fragment shader
  set_pending_run_request(std::string(initial_code));

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

void Viewport::set_pending_run_request(std::string&& new_code)
{
  pending_run_request_ = std::move(new_code);
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

void Viewport::prepare_new_frame(State& state, const gfx::Renderer& renderer)
{
  if (pending_run_request_.has_value()) {
    const auto& new_code = pending_run_request_.value();
    // TODO: check if the code is the same before creating new fragment shader module
    //       (how expensive is this anyway?)
    const auto& [frag_module_opt, diagnostics] = gfx::create::shader_module_from_wgsl(
        renderer, new_code, DEFAULT_FRAG_SHADER_LABEL.data());

    if (auto diag_count = diagnostics.size(); diag_count > 0) {
      std::println("Shader compilation generated {} diagnostic(s):", diag_count);

      for (const auto& diag : diagnostics) {
        std::println(
            "- ({}:{}) {}: {}", diag.line_num, diag.line_pos, diag.type_name, diag.message);
        std::println("  {}", diag.highlight);

        std::print("  ");
        for (size_t i = 0; i < diag.highlight.size(); ++i)
          std::print("^");
        std::println();
      }
    }

    if (frag_module_opt.has_value()) {
      fragment_state_.module = frag_module_opt.value();
      update_render_pipeline(renderer.device());

      if constexpr (query::is_debug())
        std::println("Updated viewport render pipeline");
    } else {
      if constexpr (query::is_debug())
        std::println("Shader compilation errors occurred, viewport render pipeline not updated");
    }

    pending_run_request_ = std::nullopt;
  }

  if (pending_resize_.has_value()) {
    auto [new_width, new_height] = pending_resize_.value();

    if constexpr (query::is_debug())
      std::println("Viewport texture resized to {}×{}", new_width, new_height);

    // TODO: on initialization a couple of intermediary resizes occur, including
    //       a strange one to a resolution of 16×9 (yes, 16 pixels by 9 pixels)
    texture_desc_.size.width = new_width;
    texture_desc_.size.height = new_height;
    texture_ = renderer.device().CreateTexture(&texture_desc_);

    static const wgpu::TextureViewDescriptor VIEW_DESC = { .label = "viewport-view" };

    view_ = texture_.CreateView(&VIEW_DESC);
    pass_color_attachment_.view = view_;

    pending_resize_ = std::nullopt;
  }

  Uniforms unif = {
    .time = static_cast<float>(SDL_GetTicksNS()) / 1'000'000'000.f,
    .resolution
    = { static_cast<float>(texture_.GetWidth()), static_cast<float>(texture_.GetHeight()) },
  };

  renderer.queue().WriteBuffer(unif_buf_, 0, &unif, sizeof(Uniforms));

  // TODO: update time in the main render loop, not within this class
  state.time = unif.time;
}

}
