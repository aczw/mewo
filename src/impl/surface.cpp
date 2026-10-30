#include "impl/surface.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "core/utility/match.hpp"

#include <webgpu/webgpu.h>

#include <algorithm>
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

std::optional<gpu::TexView> Surface::acquire(
  const gpu::Adapter& adapter,
  const gpu::Device& device
) {
  // No need to present anything this frame if we can't see anything
  if (width_ == 0 || height_ == 0 || is_invalid_)
    return std::nullopt;

  if (needs_configure_) {
    if (!configure(adapter, device))
      return std::nullopt;
    needs_configure_ = false;
  }

  WGPUSurfaceTexture st_holder = WGPU_SURFACE_TEXTURE_INIT;
  wgpuSurfaceGetCurrentTexture(surface_.get(), &st_holder);
  gpu::Tex surface_tex(st_holder.texture);

  auto create_view = [&surface_tex]() {
    WGPUTextureViewDescriptor view_desc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
    view_desc.label = gpu::to_string_view("surface-view");
    return gpu::TexView(wgpuTextureCreateView(surface_tex.get(), &view_desc));
  };

  switch (st_holder.status) {
    case WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal: return create_view();

    case WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal: {
      needs_configure_ = true;
      return create_view();
    }

    case WGPUSurfaceGetCurrentTextureStatus_Outdated:
    case WGPUSurfaceGetCurrentTextureStatus_Lost: {
      needs_configure_ = true;
      return std::nullopt;
    }

    // No texture became available in time, so skip this frame. We don't need a reconfigure
    case WGPUSurfaceGetCurrentTextureStatus_Timeout: return std::nullopt;

    default: {
      is_invalid_ = true;
      return std::nullopt;
    }
  }
}

bool Surface::configure(const gpu::Adapter& adapter, const gpu::Device& device) {
  config_.width = width_;
  config_.height = height_;

  gpu::SurfaceCaps caps(WGPU_SURFACE_CAPABILITIES_INIT);
  WGPUStatus status = wgpuSurfaceGetCapabilities(surface_.get(), adapter.get(), &caps.get());
  const WGPUSurfaceCapabilities& surface_caps = caps.get();

  if (status != WGPUStatus_Success || surface_caps.formatCount == 0) {
    is_invalid_ = true;
    return false;
  }

  auto has_format = [&surface_caps](WGPUTextureFormat format) {
    const WGPUTextureFormat* end = surface_caps.formats + surface_caps.formatCount;
    return std::find(surface_caps.formats, end, format) != end;
  };

  if (has_format(WGPUTextureFormat_BGRA8Unorm)) {
    config_.format = WGPUTextureFormat_BGRA8Unorm;
  } else if (has_format(WGPUTextureFormat_RGBA8Unorm)) {
    config_.format = WGPUTextureFormat_RGBA8Unorm;
  } else {
    config_.format = surface_caps.formats[0];
  }

  config_.usage = WGPUTextureUsage_RenderAttachment;
  config_.device = device.get();
  config_.presentMode = WGPUPresentMode_Fifo;

  // Prefer an opaque alpha mode
  config_.alphaMode = [&surface_caps]() -> WGPUCompositeAlphaMode {
    const WGPUCompositeAlphaMode* end = surface_caps.alphaModes + surface_caps.alphaModeCount;
    if (std::find(surface_caps.alphaModes, end, WGPUCompositeAlphaMode_Opaque) != end) {
      return WGPUCompositeAlphaMode_Opaque;
    } else {
      return WGPUCompositeAlphaMode_Auto;
    }
  }();

  wgpuSurfaceConfigure(surface_.get(), &config_);

  return true;
}

}  // namespace mewo::impl
