#pragma once

#include "event/event_queue.hpp"
#include "frame_context.hpp"
#include "window.hpp"

#include <webgpu/webgpu_cpp.h>

#include <limits>

namespace mewo::gfx {

class Gfx {
 public:
  static constexpr auto WAIT_TIMEOUT_MAX = std::numeric_limits<uint64_t>::max();

  Gfx(EventQueue& event_queue, const Window& window);

  ~Gfx() { surface_.Unconfigure(); }

  Gfx(const Gfx&) = delete;
  Gfx& operator=(const Gfx&) = delete;

  const wgpu::Instance& instance() const { return instance_; }

  const wgpu::Device& device() const { return device_; }

  const wgpu::Surface& surface() const { return surface_; }

  const wgpu::SurfaceConfiguration& surface_config() const { return surface_config_; }

  const wgpu::Queue& queue() const { return queue_; }

  /// Checks if any errors have occurred in the graphics context, and throws accordingly.
  /// Otherwise, it returns a texture view of the current surface and a new command encoder.
  const FrameContext begin_frame();

  /// Consumes the given command encoder and submits the corresponding command buffer
  /// to the queue. We then request surface presentation and tick the device.
  void end_frame(const FrameContext&& frame_ctx) const;

  void resize_surface(uint32_t new_width, uint32_t new_height) {
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

  uint64_t frame_count_ = 0;
};

}  // namespace mewo::gfx
