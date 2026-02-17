#include "out.hpp"

#include "gfx/create.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

#include <filesystem>
#include <string_view>

namespace mewo {

namespace {

constexpr std::string_view WGPU_ENTRY_POINT = "main";

#if defined(MEWO_IS_DEBUG)
constexpr std::string_view OUT_VERT_SHADER_FILE_PATH = "../../assets/shaders/out.vert.wgsl";
#else
#error "TODO: handle "out.vert.wgsl" file path on release mode"
constexpr std::string_view OUT_VERT_SHADER_FILE_PATH = "out.vert.wgsl";
#endif

}

Out::Out(const gfx::Renderer& renderer, std::string initial_code)
{
  const wgpu::Device& device = renderer.device();
  const wgpu::SurfaceConfiguration& surface_config = renderer.surface_config();

  color_target_state_ = { .format = surface_config.format };

  render_pipeline_desc_ = {
    .label = "out-render-pipeline",
    .vertex = { .module = gfx::create::shader_module_from_wgsl(device,
                    std::filesystem::path(OUT_VERT_SHADER_FILE_PATH), "out-vert-shader-module"),
        .entryPoint = WGPU_ENTRY_POINT },
  };

  set_fragment_shader(device, initial_code);

  wgpu::TextureDescriptor texture_desc = {
    .label = "out-texture",
    .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
    .size = { .width = surface_config.width, .height = surface_config.height },
    .format = surface_config.format,
  };

  texture_ = device.CreateTexture(&texture_desc);

  wgpu::TextureViewDescriptor view_desc = { .label = "out-view" };

  view_ = texture_.CreateView(&view_desc);
}

const wgpu::TextureView& Out::view() const { return view_; }

Out::DisplayMode Out::display_mode() const { return display_mode_; }

AspectRatio::Preset Out::aspect_ratio_preset() const { return aspect_ratio_preset_; }

void Out::set_display_mode(DisplayMode display_mode) { display_mode_ = display_mode; }

void Out::set_aspect_ratio_preset(AspectRatio::Preset preset) { aspect_ratio_preset_ = preset; }

void Out::record(const gfx::FrameContext& frame_ctx) const
{
  wgpu::RenderPassColorAttachment color_attachment = {
    .view = view_,
    .loadOp = wgpu::LoadOp::Clear,
    .storeOp = wgpu::StoreOp::Store,
  };

  wgpu::RenderPassDescriptor render_pass_desc = {
    .label = "out-render-pass",
    .colorAttachmentCount = 1,
    .colorAttachments = &color_attachment,
  };

  wgpu::RenderPassEncoder render_pass = frame_ctx.encoder.BeginRenderPass(&render_pass_desc);
  render_pass.SetPipeline(render_pipeline_);
  render_pass.Draw(6);
  render_pass.End();
}

void Out::set_fragment_shader(const wgpu::Device& device, std::string_view code)
{
  wgpu::FragmentState fragment_state = {
    .module = gfx::create::shader_module_from_wgsl(device, code, "out-frag-shader-module"),
    .entryPoint = WGPU_ENTRY_POINT,
    .targetCount = 1,
    .targets = &color_target_state_,
  };

  render_pipeline_desc_.fragment = &fragment_state;
  render_pipeline_ = device.CreateRenderPipeline(&render_pipeline_desc_);
}

}
