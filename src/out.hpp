#pragma once

#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

namespace mewo {

/// Visual output seen on the screen. Does not directly render to the surface
/// texture because it will be displayed in the GUI instead.
class Out {
  public:
  /// Determines the "shape" of the output. Regardless of the choice, it will fill the width
  /// of the current GUI window that it's in.
  enum class DisplayMode {
    AspectRatio, ///< Uses a predefined aspect ratio and does not care about pixel resolution.
    Resolution, ///< Uses a predefined pixel resolution, possibly resulting in up/downsampling.
  };

  Out(const gfx::Renderer& renderer);

  const wgpu::TextureView& view() const;

  void record(const gfx::FrameContext& frame_ctx) const;

  private:
  wgpu::RenderPipeline render_pipeline_;
  wgpu::Texture texture_;
  wgpu::TextureView view_;

  [[maybe_unused]] DisplayMode display_mode_ = DisplayMode::AspectRatio;
};

}
