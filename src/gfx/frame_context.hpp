#pragma once

#include <webgpu/webgpu_cpp.h>

namespace mewo::gfx {

struct FrameContext {
  wgpu::TextureView surface_view;
  wgpu::CommandEncoder encoder;
};

}
