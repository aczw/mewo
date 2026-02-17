#pragma once

#include "aspect_ratio.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <string_view>

namespace mewo {

/// Visual output seen on the screen. Does not directly render to the surface
/// texture because it will be displayed in the GUI instead.
class Out {
  public:
  /// Determines the "shape" of the output. Regardless of the choice, it will fill the width
  /// of the current GUI window that it's in.
  enum class DisplayMode : int {
    AspectRatio, ///< Uses a predefined aspect ratio and does not care about pixel resolution.
    Resolution, ///< Uses a predefined pixel resolution, possibly resulting in up/downsampling.
  };

  Out(const gfx::Renderer& renderer, std::string initial_code);

  const wgpu::TextureView& view() const;
  DisplayMode display_mode() const;
  AspectRatio::Preset aspect_ratio_preset() const;

  void set_display_mode(DisplayMode display_mode);
  void set_aspect_ratio_preset(AspectRatio::Preset preset);

  void record(const gfx::FrameContext& frame_ctx) const;
  void set_fragment_shader(const wgpu::Device& device, std::string_view code);

  private:
  wgpu::ColorTargetState color_target_state_;
  wgpu::RenderPipelineDescriptor render_pipeline_desc_;
  wgpu::RenderPipeline render_pipeline_;

  wgpu::Texture texture_;
  wgpu::TextureView view_;

  DisplayMode display_mode_ = DisplayMode::AspectRatio;
  AspectRatio::Preset aspect_ratio_preset_ = AspectRatio::Preset::e16_9;
};

}
