#pragma once

#include "error.hpp"
#include "sdl/window.hpp"

#include <webgpu/webgpu_cpp.h>

#include <optional>

namespace mewo::gfx {

class Renderer {
  public:
  Renderer(sdl::Window& window);
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  const wgpu::Device& device();
  const wgpu::Surface& surface();
  const wgpu::SurfaceConfiguration& surface_config();
  const wgpu::Queue& queue();

  private:
  wgpu::Instance instance_ = {};
  wgpu::Device device_ = {};
  wgpu::Surface surface_ = {};
  wgpu::SurfaceConfiguration surface_config_ = {};
  wgpu::Queue queue_ = {};

  std::optional<Error> device_lost_error_;
  std::optional<Error> uncaptured_error_;
};

}
