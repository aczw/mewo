#pragma once

#include "core/gpu/types.hpp"

#include <cstdint>
#include <expected>
#include <string>

namespace mewo::impl {

struct CompileRequestError {
  std::string message;
  WGPUCreatePipelineAsyncStatus status;
};

/**
 * This will be cast to `uintptr_t` which means if we want it available on all
 * WASM targets, it can at most be 32 bits. It should still be large enough for our purposes.
 */
using CompileRequestId = uint32_t;
using CompileRequestResult = std::expected<gpu::RenderPipeline, CompileRequestError>;

}  // namespace mewo::impl
