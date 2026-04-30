#pragma once

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace mewo::gfx {

struct FrameContext {
  wgpu::TextureView surface_view;
  wgpu::CommandEncoder encoder;
  uint32_t number = 0;
};

}  // namespace mewo::gfx
