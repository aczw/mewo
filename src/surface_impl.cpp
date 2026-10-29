#include "surface_impl.hpp"

#include "core/gpu_types.hpp"
#include "core/utility/match.hpp"

#include <webgpu/webgpu.h>

#include <memory>
#include <optional>
#include <utility>
#include <variant>

namespace mewo {

std::optional<std::unique_ptr<SurfaceImpl>> SurfaceImpl::create(
  const Instance& instance,
  uint32_t initial_width,
  uint32_t initial_height,
  const Source& source
) {
  WGPUSurfaceDescriptor desc = WGPU_SURFACE_DESCRIPTOR_INIT;

  Surface surface = std::visit(
    Match{
      [&desc, &instance](const MetalLayer& variant) {
        WGPUSurfaceSourceMetalLayer ml = WGPU_SURFACE_SOURCE_METAL_LAYER_INIT;
        ml.layer = variant.layer;

        desc.nextInChain = &ml.chain;
        return Surface(wgpuInstanceCreateSurface(instance.get(), &desc));
      },
      [&desc, &instance](const Win32& variant) {
        WGPUSurfaceSourceWindowsHWND win = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;
        win.hinstance = variant.hinstance;
        win.hwnd = variant.hwnd;

        desc.nextInChain = &win.chain;
        return Surface(wgpuInstanceCreateSurface(instance.get(), &desc));
      }
    },
    source
  );

  if (!surface)
    return std::nullopt;

  return std::unique_ptr<SurfaceImpl>(
    new SurfaceImpl(std::move(surface), initial_width, initial_height)
  );
}

void SurfaceImpl::resize(uint32_t new_width, uint32_t new_height) {
  width_ = new_width;
  height_ = new_height;
  needs_configure_ = true;
}

}  // namespace mewo
