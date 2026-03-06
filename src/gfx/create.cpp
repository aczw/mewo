#include "create.hpp"

#include "exception.hpp"
#include "gfx/compilation_diagnostic.hpp"
#include "gfx/renderer.hpp"
#include "utility.hpp"

#include <webgpu/webgpu_cpp.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mewo::gfx::create {

static std::string_view get_compilation_mesage_type(wgpu::CompilationMessageType msg_type)
{
  switch (msg_type) {
    // clang-format off
  case wgpu::CompilationMessageType::Error: return "error";
  case wgpu::CompilationMessageType::Warning: return "warning";
  case wgpu::CompilationMessageType::Info: return "info";
    // clang-format on

  default:
    utility::enum_unreachable("wgpu::CompilationMessageType", msg_type);
  }
}

ShaderCompilationResult shader_module_from_wgsl(
    const Renderer& renderer, std::string_view code, std::string_view label)
{
  wgpu::ShaderSourceWGSL shader_source_wgsl = { { .code = code } };

  wgpu::ShaderModuleDescriptor shader_module_desc = {
    .nextInChain = &shader_source_wgsl,
    .label = label,
  };

  wgpu::ShaderModule shader = renderer.device().CreateShaderModule(&shader_module_desc);
  std::vector<CompilationDiagnostic> diagnostics;
  bool did_error_occur = false;

  wgpu::WaitStatus shader_status = renderer.instance().WaitAny(
      shader.GetCompilationInfo(wgpu::CallbackMode::WaitAnyOnly,
          [&diagnostics, &did_error_occur, &code](
              wgpu::CompilationInfoRequestStatus status, const wgpu::CompilationInfo* info) {
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
                  .line_num = msg.lineNum,
                  .line_pos = msg.linePos,
                  .highlight = std::string(code.substr(msg.offset, msg.length)),
              });
            }
          }),
      Renderer::WAIT_TIMEOUT_MAX);

  if (shader_status != wgpu::WaitStatus::Success)
    throw Exception("Waiting on wgpu::ShaderModule::GetCompilationInfo failed");

  return { did_error_occur ? std::nullopt : std::optional(shader), diagnostics };
}

}
