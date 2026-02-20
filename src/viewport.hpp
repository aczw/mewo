#pragma once

#include "aspect_ratio.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>

namespace mewo {

/// Handles the final visual output seen on screen. Does not directly render to the surface
/// texture. Instead, it renders to a custom texture that the GUI will display as an image.
///
/// This class does not own the fragment shader, and only uses it to set up the graphics pipeline.
/// Therefore it only takes views to any references of code.
class Viewport {
  public:
  /// Affects the shape of the output as well as how the underlying texture is updated, like
  /// when a window or panel is resized.
  enum class Mode : int {
    /// Uses a ratio (w:h). Resolution is derived from the width of its GUI panel.
    AspectRatio,
    /// Uses a resolution (w×h). Displayed texture in the GUI may undergo up or downsampling.
    Resolution,
  };

  /// A pending resize can specify a new width and optionally a new height.
  using PendingSize = std::pair<uint32_t, std::optional<uint32_t>>;

  Viewport(const gfx::Renderer& renderer, std::string_view initial_code);

  const wgpu::TextureView& view() const;
  Mode mode() const;
  AspectRatio::Preset ratio_preset() const;
  uint32_t width() const;
  uint32_t height() const;

  void set_fragment_state(const wgpu::Device& device, std::string_view code);
  void set_mode(Mode display_mode);
  void set_ratio_preset(AspectRatio::Preset preset);
  void set_pending_size(PendingSize pending_size);

  void record(const gfx::FrameContext& frame_ctx) const;
  /// Updates the fragment shader and creates the render pipeline.
  void update_render_pipeline(const wgpu::Device& device);
  void resize_with_ratio_preset(const wgpu::Device& device, uint32_t gui_window_width);
  void resize_with_resolution(const wgpu::Device& device, uint32_t new_width, uint32_t new_height);

  private:
  /// Internal helper that receives the new width/height and creates the new texture/view.
  void resize(const wgpu::Device& device, uint32_t new_width, uint32_t new_height);

  wgpu::ColorTargetState color_target_state_;
  wgpu::FragmentState fragment_state_;
  wgpu::RenderPipelineDescriptor render_pipeline_desc_;
  wgpu::RenderPipeline render_pipeline_;

  wgpu::RenderPassColorAttachment pass_color_attachment_;
  wgpu::RenderPassDescriptor pass_desc_;

  wgpu::TextureDescriptor texture_desc_;
  wgpu::Texture texture_;
  wgpu::TextureView view_;

  Mode mode_ = Mode::AspectRatio;
  AspectRatio::Preset ratio_preset_ = AspectRatio::Preset::e16_9;
  uint32_t width_ = 0;
  uint32_t height_ = 0;

  /// Stores the pending texture resize that will be applied next frame. Populated while
  /// building the UI, and consumed in `Viewport::check_for_resize`.
  std::optional<PendingSize> pending_size_;
};

}
