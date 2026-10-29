#pragma once

#include "gpu/move_only_handle.hpp"

#include <webgpu/webgpu.h>

namespace mewo {

using Instance = MoveOnlyHandle<WGPUInstance, wgpuInstanceRelease>;
using Adapter = MoveOnlyHandle<WGPUAdapter, wgpuAdapterRelease>;
using Device = MoveOnlyHandle<WGPUDevice, wgpuDeviceRelease>;

}  // namespace mewo
