#include "out.hpp"

#include <webgpu/webgpu_cpp.h>

namespace mewo {

void Out::record(const gfx::FrameContext& frame_ctx) const
{
  auto& [surface_view, encoder] = frame_ctx;

  wgpu::RenderPassColorAttachment color_attachment = {
    .view = surface_view,
    .loadOp = wgpu::LoadOp::Clear,
    .storeOp = wgpu::StoreOp::Store,
    .clearValue = wgpu::Color { .r = 1.0, .g = 0.0, .b = 1.0, .a = 1.0 },
  };

  wgpu::RenderPassDescriptor render_pass_desc = {
    .label = "out-render-pass",
    .colorAttachmentCount = 1,
    .colorAttachments = &color_attachment,
  };

  wgpu::RenderPassEncoder render_pass = encoder.BeginRenderPass(&render_pass_desc);
  render_pass.End();
}

}
