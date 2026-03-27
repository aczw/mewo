#pragma once

#include "error.hpp"
#include "frame_context.hpp"
#include "sdl/window.hpp"

#include <webgpu/webgpu_cpp.h>

#include <limits>
#include <optional>

namespace mewo::gfx {

class Renderer {
 public:
  static constexpr auto WAIT_TIMEOUT_MAX = std::numeric_limits<uint64_t>::max();

  Renderer(const sdl::Window& window);

  ~Renderer() { surface_.Unconfigure(); }

  Renderer(const Renderer&) = delete;

  Renderer& operator=(const Renderer&) = delete;

  const wgpu::Instance& instance() const { return instance_; }

  const wgpu::Device& device() const { return device_; }

  const wgpu::Surface& surface() const { return surface_; }

  const wgpu::SurfaceConfiguration& surface_config() const { return surface_config_; }

  const wgpu::Queue& queue() const { return queue_; }

  /// Checks if any errors have occurred in the graphics context, and throws accordingly.
  /// Otherwise, it returns a texture view of the current surface and a new command encoder.
  FrameContext prepare_new_frame();

  void resize(uint32_t new_width, uint32_t new_height) {
    surface_config_.width = new_width;
    surface_config_.height = new_height;
    surface_.Configure(&surface_config_);
  }

 private:
  wgpu::Instance instance_;
  wgpu::Device device_;
  wgpu::Surface surface_;
  wgpu::SurfaceConfiguration surface_config_;
  wgpu::Queue queue_;

  // TODO: move these two fields to `Pending` struct? Would then have to deal with
  // potential concurrent writes to the object as these errors can happen at any time
  std::optional<Error> device_lost_error_;
  std::optional<Error> uncaptured_error_;
};

}  // namespace mewo::gfx
