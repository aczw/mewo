#include "viewport.hpp"

#include "aspect_ratio.hpp"
#include "exception.hpp"
#include "gfx/create.hpp"
#include "gfx/renderer.hpp"
#include "gui/layout.hpp"
#include "query.hpp"
#include "utility.hpp"

#include <webgpu/webgpu_cpp.h>

#include <cmath>
#include <filesystem>
#include <functional>
#include <optional>
#include <print>
#include <string_view>

namespace mewo {

Viewport::Viewport(const gfx::Renderer& renderer, std::string_view initial_code)
{
  const wgpu::Device& device = renderer.device();
  const wgpu::SurfaceConfiguration& surface_config = renderer.surface_config();

  color_target_state_ = { .format = surface_config.format };

  auto vert_shader_file_path = std::invoke([] -> std::filesystem::path {
    if constexpr (query::is_debug()) {
      return "../../assets/shaders/out.vert.wgsl";
    } else {
      throw Exception("TODO: handle \"out.vert.wgsl\" file path on release mode");
    }
  });

  render_pipeline_desc_ = {
    .label = "out-render-pipeline",
    .vertex = { .module = gfx::create::shader_module_from_wgsl(
                    device, vert_shader_file_path, "out-vert-shader-module"),
        .entryPoint = "main", },
  };

  set_fragment_state(device, initial_code);
  update(device);

  texture_desc_ = {
    .label = "out-texture",
    .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
    .format = surface_config.format,
  };

  auto width = std::floor(static_cast<float>(surface_config.width) * gui::Layout::SPLIT_LEFT_RATIO);
  auto height = std::floor(width * AspectRatio::get_inverse_value(ratio_preset_));
  auto width_int = static_cast<uint32_t>(width);
  auto height_int = static_cast<uint32_t>(height);

  resize(device, width_int, height_int);

  // Use the width and height from the aspect ratio preset as initial values
  width_ = width_int;
  height_ = height_int;

  pass_color_attachment_ = {
    .view = view_,
    .loadOp = wgpu::LoadOp::Clear,
    .storeOp = wgpu::StoreOp::Store,
  };

  pass_desc_ = {
    .label = "out-render-pass",
    .colorAttachmentCount = 1,
    .colorAttachments = &pass_color_attachment_,
  };
}

const wgpu::TextureView& Viewport::view() const { return view_; }

Viewport::Mode Viewport::mode() const { return mode_; }

AspectRatio::Preset Viewport::ratio_preset() const { return ratio_preset_; }

void Viewport::set_fragment_state(const wgpu::Device& device, std::string_view code)
{
  fragment_state_ = {
    .module = gfx::create::shader_module_from_wgsl(device, code, "out-frag-shader-module"),
    .entryPoint = "main",
    .targetCount = 1,
    .targets = &color_target_state_,
  };
}

void Viewport::set_mode(Mode mode) { mode_ = mode; }

void Viewport::set_ratio_preset(AspectRatio::Preset preset) { ratio_preset_ = preset; }

void Viewport::set_pending_size(PendingSize pending_size) { pending_size_ = pending_size; }

void Viewport::record(const gfx::FrameContext& frame_ctx) const
{
  wgpu::RenderPassEncoder render_pass = frame_ctx.encoder.BeginRenderPass(&pass_desc_);

  render_pass.SetPipeline(render_pipeline_);
  render_pass.Draw(6);

  render_pass.End();
}

void Viewport::update(const wgpu::Device& device)
{
  render_pipeline_desc_.fragment = &fragment_state_;
  render_pipeline_ = device.CreateRenderPipeline(&render_pipeline_desc_);
}

void Viewport::resize(const wgpu::Device& device, uint32_t new_width, uint32_t new_height)
{
  if constexpr (query::is_debug())
    std::println("Viewport texture resized to {}×{}", new_width, new_height);

  texture_desc_.size.width = new_width;
  texture_desc_.size.height = new_height;
  texture_ = device.CreateTexture(&texture_desc_);

  static const wgpu::TextureViewDescriptor VIEW_DESC = { .label = "out-view" };

  view_ = texture_.CreateView(&VIEW_DESC);
  pass_color_attachment_.view = view_;
}

void Viewport::check_for_resize(const wgpu::Device& device)
{
  if (!pending_size_.has_value())
    return;

  // TODO: after the initial viewport texture size, a pending texture size of "16×9" is
  //       received. Should probably get rid of that
  auto new_size = std::invoke([this] -> std::pair<uint32_t, uint32_t> {
    switch (mode_) {
    case Mode::AspectRatio: {
      auto width = pending_size_->first;
      auto height = static_cast<float>(width) * AspectRatio::get_inverse_value(ratio_preset_);
      return { width, static_cast<uint32_t>(std::floor(height)) };
    }

    case Mode::Resolution:
      throw Exception("TODO: implement resizing operation for resolution");

    default:
      utility::enum_unreachable("Viewport::Mode", mode_);
    }
  });

  resize(device, new_size.first, new_size.second);
  pending_size_ = std::nullopt;
}

float Viewport::current_inverse_ratio() const
{
  switch (mode_) {
  case Mode::AspectRatio:
    return AspectRatio::get_inverse_value(ratio_preset_);
  case Mode::Resolution:
    // TODO: division by zero possible
    return static_cast<float>(height_) / static_cast<float>(width_);

  default:
    utility::enum_unreachable("Viewport::Mode", mode_);
  }
}

}
