#pragma once

#include "core/compilation/diagnostic.hpp"
#include "core/compilation/line.hpp"
#include "core/gpu/types.hpp"

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace mewo::impl {

/**
 * This will be cast to `uintptr_t` which means if we want it available on all
 * WASM targets, it can at most be 32 bits. It should still be large enough for our purposes.
 */
using CompileRequestId = uint32_t;

/**
 * Maps to WebGPU's `WGPUCompilationMessage` type.
 */
struct CompileMessage {
  Severity severity;
  std::string message;
  LineNum line_number;
  LineCol line_column;
};

struct CompileError {
  std::string message;
  WGPUCreatePipelineAsyncStatus status;
};

using CompileResult = std::expected<gpu::RenderPipeline, CompileError>;

struct CompileEntry {
  /**
   * Filled in by `wgpuDeviceCreateRenderPipelineAsync`.
   */
  std::optional<CompileResult> result;

  /**
   * Filled in by `wgpuShaderModuleGetCompilationInfo`.
   */
  std::optional<std::vector<CompileMessage>> messages;

  /**
   * A compilation can be discarded if it was in-flight when the user changed the shader.
   * In this case, this compilation has become outdated and won't be used anymore.
   */
  bool discarded;
};

}  // namespace mewo::impl
