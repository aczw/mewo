#pragma once

#include <webgpu/webgpu_cpp.h>

namespace mewo::gfx {

struct FrameContext {
  wgpu::TextureView surface_view;
  wgpu::CommandEncoder encoder;
  uint64_t number = 0;
};

}  // namespace mewo::gfx
