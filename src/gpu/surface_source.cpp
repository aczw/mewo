#include "gpu/surface_source.hpp"

#include "gpu/types.hpp"
#include "utility/match.hpp"

#include <webgpu/webgpu.h>

#include <variant>

namespace mewo {

Surface create_surface_from_source(const Instance& instance, const SurfaceSource& source) {
  WGPUSurfaceDescriptor desc = WGPU_SURFACE_DESCRIPTOR_INIT;

  return std::visit(
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
}

}  // namespace mewo
