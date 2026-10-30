#include "impl/surface.hpp"

#include "core/gpu/types.hpp"
#include "core/utility/match.hpp"

#include <webgpu/webgpu.h>

#include <memory>
#include <optional>
#include <utility>
#include <variant>

namespace mewo::impl {

std::optional<std::unique_ptr<Surface>> Surface::create(
  const gpu::Instance& instance,
  uint32_t initial_width,
  uint32_t initial_height,
  const SurfaceSource& source
) {
  WGPUSurfaceDescriptor desc = WGPU_SURFACE_DESCRIPTOR_INIT;

  gpu::Surface surface = std::visit(
    Match{
      [&desc, &instance](const MetalLayer& variant) {
        WGPUSurfaceSourceMetalLayer ml = WGPU_SURFACE_SOURCE_METAL_LAYER_INIT;
        ml.layer = variant.layer;

        desc.nextInChain = &ml.chain;
        return gpu::Surface(wgpuInstanceCreateSurface(instance.get(), &desc));
      },
      [&desc, &instance](const Win32& variant) {
        WGPUSurfaceSourceWindowsHWND win = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;
        win.hinstance = variant.hinstance;
        win.hwnd = variant.hwnd;

        desc.nextInChain = &win.chain;
        return gpu::Surface(wgpuInstanceCreateSurface(instance.get(), &desc));
      }
    },
    source
  );

  if (!surface)
    return std::nullopt;

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Surface>(new Surface(std::move(surface), initial_width, initial_height));
}

void Surface::resize(uint32_t new_width, uint32_t new_height) {
  width_ = new_width;
  height_ = new_height;
  needs_configure_ = true;
}

}  // namespace mewo::impl
