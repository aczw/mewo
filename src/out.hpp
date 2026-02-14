#pragma once

#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

namespace mewo {

/// Visual output seen on the screen.
class Out {
  public:
  Out(const gfx::Renderer& renderer);

  void record(const gfx::FrameContext& frame_ctx) const;

  private:
  wgpu::RenderPipeline render_pipeline_;
};

}
