#include "out.hpp"

#include "fs.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

#include <functional>
#include <string_view>

namespace mewo {

namespace {

constexpr wgpu::BlendState WGPU_DEFAULT_BLEND_STATE = {};
constexpr std::string_view WGPU_ENTRY_POINT = "main";

#if defined(MEWO_IS_DEBUG)
constexpr std::string_view OUT_VERT_SHADER_FILE_PATH = "../../assets/shaders/out.vert.wgsl";
constexpr std::string_view OUT_FRAG_SHADER_FILE_PATH = "../../assets/shaders/out.frag.wgsl";
#else
#error "TODO: handle "out.{frag,vert}.wgsl" file path on release mode"
constexpr std::string_view OUT_VERT_SHADER_FILE_PATH = "out.vert.wgsl";
constexpr std::string_view OUT_FRAG_SHADER_FILE_PATH = "out.frag.wgsl";
#endif

}

Out::Out(const gfx::Renderer& renderer)
{
  const wgpu::SurfaceConfiguration& surface_config = renderer.surface_config();

  wgpu::ColorTargetState surface_color_target_state = {
    .format = surface_config.format,
    // Has to be a valid pointer because blending is disabled by default
    .blend = &WGPU_DEFAULT_BLEND_STATE,
  };

  auto out_frag_shader_module = std::invoke([&device = renderer.device()] -> wgpu::ShaderModule {
    std::string out_frag_shader_src = fs::read_wgsl_shader(OUT_FRAG_SHADER_FILE_PATH);

    wgpu::ShaderSourceWGSL out_frag_shader_wgsl = { { .code = out_frag_shader_src.c_str() } };
    wgpu::ShaderModuleDescriptor out_frag_shader_module_desc
        = { .nextInChain = &out_frag_shader_wgsl, .label = "out-frag-shader-module" };

    return device.CreateShaderModule(&out_frag_shader_module_desc);
  });

  wgpu::FragmentState fragment_state = {
    .module = out_frag_shader_module,
    .entryPoint = WGPU_ENTRY_POINT,
    .targetCount = 1,
    .targets = &surface_color_target_state,
  };

  auto out_vert_shader_module = std::invoke([&device = renderer.device()] -> wgpu::ShaderModule {
    std::string out_vert_shader_src = fs::read_wgsl_shader(OUT_VERT_SHADER_FILE_PATH);

    wgpu::ShaderSourceWGSL out_vert_shader_wgsl = { { .code = out_vert_shader_src.c_str() } };
    wgpu::ShaderModuleDescriptor out_vert_shader_module_desc
        = { .nextInChain = &out_vert_shader_wgsl, .label = "out-vert-shader-module" };

    return device.CreateShaderModule(&out_vert_shader_module_desc);
  });

  wgpu::RenderPipelineDescriptor render_pipeline_desc = {
    .label = "out-render-pipeline",
    .vertex = { .module = out_vert_shader_module, .entryPoint = WGPU_ENTRY_POINT },
    .fragment = &fragment_state,
  };

  render_pipeline_ = renderer.device().CreateRenderPipeline(&render_pipeline_desc);

  wgpu::TextureDescriptor texture_desc = {
    .label = "out-texture",
    .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
    .size = { .width = surface_config.width, .height = surface_config.height },
    .format = surface_config.format,
  };

  texture_ = renderer.device().CreateTexture(&texture_desc);

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

}
