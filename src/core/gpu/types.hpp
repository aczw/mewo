#pragma once

#include <webgpu/webgpu.h>

#include <type_traits>
#include <utility>

namespace mewo::gpu {

template <typename T, void (*release_fn)(T)>
  requires std::is_pointer_v<T>
class RaiiHandle {
 public:
  RaiiHandle() = default;

  explicit RaiiHandle(T handle) : handle_(handle) {}

  ~RaiiHandle() {
    if (handle_)
      release_fn(handle_);
  }

  RaiiHandle(RaiiHandle&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}

  RaiiHandle& operator=(RaiiHandle&& other) noexcept {
    if (this != &other) {
      if (handle_)
        release_fn(handle_);

      handle_ = std::exchange(other.handle_, nullptr);
    }

    return *this;
  }

  RaiiHandle(const RaiiHandle&) = delete;
  RaiiHandle& operator=(const RaiiHandle&) = delete;

  T get() const { return handle_; }

  explicit operator bool() const { return handle_ != nullptr; }

 private:
  T handle_ = nullptr;
};

template <typename T, void (*free_fn)(T)>
  requires std::is_class_v<T>
class Raii {
 public:
  explicit Raii(T value) : value_(value) {}

  ~Raii() { free_fn(value_); }

  Raii(Raii&&) = delete;
  Raii& operator=(Raii&&) = delete;
  Raii(const Raii&) = delete;
  Raii& operator=(const Raii&) = delete;

  T& get() { return value_; }

  const T& get() const { return value_; }

 private:
  T value_;
};

using Instance = RaiiHandle<WGPUInstance, wgpuInstanceRelease>;
using Adapter = RaiiHandle<WGPUAdapter, wgpuAdapterRelease>;
using Device = RaiiHandle<WGPUDevice, wgpuDeviceRelease>;
using Surface = RaiiHandle<WGPUSurface, wgpuSurfaceRelease>;
using SurfaceCaps = Raii<WGPUSurfaceCapabilities, wgpuSurfaceCapabilitiesFreeMembers>;
using Tex = RaiiHandle<WGPUTexture, wgpuTextureRelease>;
using TexView = RaiiHandle<WGPUTextureView, wgpuTextureViewRelease>;

}  // namespace mewo::gpu
