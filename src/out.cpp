#include "out.hpp"

#include "fs.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

#include <string_view>

namespace mewo {

namespace {

constexpr wgpu::BlendState WGPU_DEFAULT_BLEND_STATE = {};
constexpr std::string_view WGPU_VERT_ENTRY_POINT = "vs_main";
constexpr std::string_view WGPU_FRAG_ENTRY_POINT = "fs_main";

#if defined(MEWO_IS_DEBUG)
constexpr std::string_view OUT_SHADER_FILE_PATH = "../../assets/shaders/out.wgsl";
#else
#error "TODO: handle "out.wgsl" file path on release mode"
constexpr std::string_view OUT_SHADER_FILE_PATH = "out.wgsl";
#endif

}

Out::Out(const gfx::Renderer& renderer)
{
  std::string out_shader_src = fs::read_wgsl_shader(OUT_SHADER_FILE_PATH);

  wgpu::ShaderSourceWGSL out_shader_src_wgsl = { { .code = out_shader_src.c_str() } };
  wgpu::ShaderModuleDescriptor out_shader_module_desc
      = { .nextInChain = &out_shader_src_wgsl, .label = "out-shader-module" };
  wgpu::ShaderModule out_shader_module
      = renderer.device().CreateShaderModule(&out_shader_module_desc);

  wgpu::ColorTargetState surface_color_target_state = {
    .format = renderer.surface_config().format,
    // Has to be a valid pointer because blending is disabled by default
    .blend = &WGPU_DEFAULT_BLEND_STATE,
  };

  wgpu::FragmentState fragment_state = {
    .module = out_shader_module,
    .entryPoint = WGPU_FRAG_ENTRY_POINT,
    .targetCount = 1,
    .targets = &surface_color_target_state,
  };

  wgpu::RenderPipelineDescriptor render_pipeline_desc = {
    .label = "out-render-pipeline",
    .vertex = { .module = out_shader_module, .entryPoint = WGPU_VERT_ENTRY_POINT },
    .fragment = &fragment_state,
  };

  render_pipeline_ = renderer.device().CreateRenderPipeline(&render_pipeline_desc);
}

void Out::record(const gfx::FrameContext& frame_ctx) const
{
  auto& [surface_view, encoder] = frame_ctx;

  wgpu::RenderPassColorAttachment color_attachment = {
    .view = surface_view,
    .loadOp = wgpu::LoadOp::Clear,
    .storeOp = wgpu::StoreOp::Store,
  };

  wgpu::RenderPassDescriptor render_pass_desc = {
    .label = "out-render-pass",
    .colorAttachmentCount = 1,
    .colorAttachments = &color_attachment,
  };

  wgpu::RenderPassEncoder render_pass = encoder.BeginRenderPass(&render_pass_desc);
  render_pass.SetPipeline(render_pipeline_);
  render_pass.Draw(6);
  render_pass.End();
}

}
