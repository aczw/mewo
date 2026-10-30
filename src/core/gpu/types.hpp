#pragma once

#include <webgpu/webgpu.h>

#include <type_traits>
#include <utility>

namespace mewo::gpu {

template <typename T, void (*release_fn)(T)>
  requires std::is_pointer_v<T>
class MoveOnlyHandle {
 public:
  MoveOnlyHandle() = default;

  explicit MoveOnlyHandle(T handle) : handle_(handle) {}

  ~MoveOnlyHandle() {
    if (handle_)
      release_fn(handle_);
  }

  MoveOnlyHandle(MoveOnlyHandle&& other) noexcept
      : handle_(std::exchange(other.handle_, nullptr)) {}

  MoveOnlyHandle& operator=(MoveOnlyHandle&& other) noexcept {
    if (this != &other) {
      if (handle_)
        release_fn(handle_);

      handle_ = std::exchange(other.handle_, nullptr);
    }

    return *this;
  }

  MoveOnlyHandle(const MoveOnlyHandle&) = delete;
  MoveOnlyHandle& operator=(const MoveOnlyHandle&) = delete;

  T get() const { return handle_; }

  explicit operator bool() const { return handle_ != nullptr; }

 private:
  T handle_ = nullptr;
};

using Instance = MoveOnlyHandle<WGPUInstance, wgpuInstanceRelease>;
using Adapter = MoveOnlyHandle<WGPUAdapter, wgpuAdapterRelease>;
using Device = MoveOnlyHandle<WGPUDevice, wgpuDeviceRelease>;
using Surface = MoveOnlyHandle<WGPUSurface, wgpuSurfaceRelease>;

}  // namespace mewo::gpu
