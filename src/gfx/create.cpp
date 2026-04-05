#include "create.hpp"

#include "exception.hpp"
#include "gfx/compilation_diagnostic.hpp"
#include "gfx/gfx.hpp"
#include "util/enum_unreachable.hpp"

#include <webgpu/webgpu_cpp.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace mewo::gfx::create {

namespace {

std::string_view get_compilation_mesage_type(wgpu::CompilationMessageType msg_type) {
  switch (msg_type) {
    case wgpu::CompilationMessageType::Error: return "error";
    case wgpu::CompilationMessageType::Warning: return "warning";
    case wgpu::CompilationMessageType::Info: return "info";

    default: util::enum_unreachable("wgpu::CompilationMessageType", msg_type);
  }
}

}  // namespace

ShaderCompilationResult shader_module_from_wgsl(
  const Gfx& gfx,
  std::string_view code,
  std::string_view label
) {
  wgpu::ShaderSourceWGSL shader_source_wgsl = {{.code = code}};

  wgpu::ShaderModuleDescriptor shader_module_desc = {
    .nextInChain = &shader_source_wgsl,
    .label = label,
  };

  wgpu::ShaderModule shader = gfx.device().CreateShaderModule(&shader_module_desc);
  gfx::CompilationDiagnostics diagnostics;
  bool did_error_occur = false;

  wgpu::WaitStatus shader_status = gfx.instance().WaitAny(
    shader.GetCompilationInfo(
      wgpu::CallbackMode::WaitAnyOnly,
      [&diagnostics,
       &did_error_occur,
       &code](wgpu::CompilationInfoRequestStatus status, const wgpu::CompilationInfo* info) {
        if (status != wgpu::CompilationInfoRequestStatus::Success)
          throw Exception("Failed to request shader compilation info");

        auto count = info->messageCount;
        diagnostics.reserve(count);

        for (size_t idx = 0; idx < count; ++idx) {
          const wgpu::CompilationMessage& msg = info->messages[idx];

          did_error_occur |= msg.type == wgpu::CompilationMessageType::Error;
          diagnostics.push_back({
            .message = std::string(msg.message),
            .type_name = get_compilation_mesage_type(msg.type),
            .line_number = msg.lineNum,
            .line_column = msg.linePos,
            .highlight = std::string(code.substr(msg.offset, msg.length)),
          });
        }
      }
    ),
    Gfx::WAIT_TIMEOUT_MAX
  );

  if (shader_status != wgpu::WaitStatus::Success)
    throw Exception("Waiting on wgpu::ShaderModule::GetCompilationInfo failed");

  return {did_error_occur ? std::nullopt : std::optional(shader), diagnostics};
}

}  // namespace mewo::gfx::create
