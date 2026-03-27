#pragma once

#include "aspect_ratio.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"
#include "pending.hpp"

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <string_view>

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

  static constexpr std::string_view FRAGMENT_SHADER_LABEL = "viewport-frag-shader";

  Viewport(
    Pending& pending,
    const std::filesystem::path& assets_dir,
    const gfx::Renderer& renderer,
    std::string_view initial_code
  );

  const wgpu::TextureView& view() const { return view_; };

  Mode mode() const { return mode_; }

  AspectRatio::Preset ratio_preset() const { return ratio_preset_; }

  uint32_t width() const { return width_; }

  uint32_t height() const { return height_; }

  void set_mode(Mode mode) { mode_ = mode; }

  void set_ratio_preset(AspectRatio::Preset ratio_preset) { ratio_preset_ = ratio_preset; }

  void set_resolution(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
  }

  void record(
    const wgpu::Queue& queue,
    const gfx::FrameContext& frame_ctx,
    float current_time
  ) const;

  void update(const wgpu::ShaderModule& fragment_module, const wgpu::Device& device);

  void resize(const wgpu::Device& device, uint32_t new_width, uint32_t new_height);

 private:
  struct alignas(8) Uniforms {
    float time = 0;
    std::array<float, 2> resolution = {};
  };

  wgpu::Buffer unif_buf_;

  wgpu::BindGroupLayout render_pipeline_bgl_;
  wgpu::BindGroup render_pipeline_bg_;
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
};

}  // namespace mewo
