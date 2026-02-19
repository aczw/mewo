#pragma once

#include "error.hpp"
#include "frame_context.hpp"
#include "sdl/window.hpp"

#include <webgpu/webgpu_cpp.h>

#include <optional>

namespace mewo::gfx {

class Renderer {
  public:
  Renderer(const sdl::Window& window);
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  const wgpu::Device& device() const;
  const wgpu::Surface& surface() const;
  const wgpu::SurfaceConfiguration& surface_config() const;
  const wgpu::Queue& queue() const;

  /// Checks if any errors have occurred in the graphics context, and throws accordingly.
  /// Otherwise, it returns a texture view of the current surface and a new command encoder.
  FrameContext prepare_new_frame() const;
  void resize(uint32_t new_width, uint32_t new_height);

  private:
  wgpu::Instance instance_;
  wgpu::Device device_;
  wgpu::Surface surface_;
  wgpu::SurfaceConfiguration surface_config_;
  wgpu::Queue queue_;

  // TODO: move these two fields to `State` struct?
  std::optional<Error> device_lost_error_;
  std::optional<Error> uncaptured_error_;
};

}
