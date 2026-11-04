#pragma once

#include "core/compilation/diagnostic.hpp"
#include "core/compilation/line.hpp"
#include "core/gpu/types.hpp"
#include "core/shader/assembler.hpp"

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

/** Maps to WebGPU's `WGPUCompilationMessage` type. */
struct CompileMessage {
  Severity severity;
  std::string message;
  LineNum line_number;
  LineCol line_column;
};

struct PipelineError {
  std::string message;
};

using PipelineResult = std::expected<gpu::RenderPipeline, PipelineError>;

struct CompileEntry {
  /** Filled in by `wgpuDeviceCreateRenderPipelineAsync`. */
  std::optional<PipelineResult> pipeline_result;
  /** Filled in by `wgpuShaderModuleGetCompilationInfo`. */
  std::optional<std::vector<CompileMessage>> messages;
  /** This compilation's mapping of line number ranges to the shader's respective sections. */
  SegmentMap segments;
};

}  // namespace mewo::impl
