#include "impl/session/pass_state.hpp"

#include "core/common.hpp"
#include "core/compilation/diagnostic.hpp"
#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "core/shader/assembler.hpp"
#include "impl/runtime/compile_request.hpp"
#include "impl/session/pass_targets.hpp"

#include <webgpu/webgpu.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mewo::impl {

PassState::PassState(
  const gpu::Device& device,
  const gpu::BindGroupLayout& common_unifs_bgl,
  const gpu::BindGroupLayout& prev_bgl,
  const Pass& pass
)
    : pass_id_(pass.id()),
      pass_label_(std::format("render-pass (ID {})", pass.id())),
      compile_status_(Pending{}) {
  std::array<WGPUBindGroupLayout, 2> bgl_entries = {common_unifs_bgl.get(), prev_bgl.get()};

  WGPUPipelineLayoutDescriptor pipeline_layout_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
  std::string pipeline_layout_label = std::format("pass-pipeline-layout (ID {})", pass.id());
  pipeline_layout_desc.label = gpu::to_string_view(pipeline_layout_label);
  pipeline_layout_desc.bindGroupLayoutCount = bgl_entries.size();
  pipeline_layout_desc.bindGroupLayouts = bgl_entries.data();

  pipeline_layout_ =
    gpu::PipelineLayout(wgpuDeviceCreatePipelineLayout(device.get(), &pipeline_layout_desc));
}

void PassState::request_render_pipeline_build(
  Runtime& runtime,
  const Pass& pass,
  std::string_view common_source,
  Revision common_revision
) {
  assert(pass_id_ == pass.id() && "PassState rebuilt from a different Pass");

  const gpu::Device& device = runtime.device();
  AssembledSource assembled_source = assemble_pass_source(pass.source(), common_source);

  WGPUShaderSourceWGSL shader_source = WGPU_SHADER_SOURCE_WGSL_INIT;
  shader_source.code = gpu::to_string_view(assembled_source.code);
  WGPUShaderModuleDescriptor shader_module_desc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
  shader_module_desc.nextInChain = &shader_source.chain;
  std::string shader_module_label = std::format("pass-shader (ID {})", pass.id());
  shader_module_desc.label = gpu::to_string_view(shader_module_label);

  // Validation errors are handled by diagnostics so filter them out here
  wgpuDevicePushErrorScope(device.get(), WGPUErrorFilter_Validation);

  gpu::ShaderModule shader_module(wgpuDeviceCreateShaderModule(device.get(), &shader_module_desc));

  WGPUPopErrorScopeCallbackInfo pop_cb_info = WGPU_POP_ERROR_SCOPE_CALLBACK_INFO_INIT;
  pop_cb_info.mode = WGPUCallbackMode_AllowProcessEvents;
  // Swallow validation errors in the callback
  pop_cb_info.callback =
    [](WGPUPopErrorScopeStatus, WGPUErrorType, WGPUStringView, void*, void*) noexcept {};
  wgpuDevicePopErrorScope(device.get(), pop_cb_info);

  WGPURenderPipelineDescriptor render_pipeline_desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
  std::string render_pipeline_label = std::format("pass-render-pipeline (ID {})", pass.id());
  render_pipeline_desc.label = gpu::to_string_view(render_pipeline_label);
  render_pipeline_desc.layout = pipeline_layout_.get();
  render_pipeline_desc.vertex.module = shader_module.get();
  render_pipeline_desc.vertex.entryPoint = gpu::to_string_view(kVertexEntryPoint);

  WGPUColorTargetState color_target_state = WGPU_COLOR_TARGET_STATE_INIT;
  color_target_state.format = PassTargets::kTexFormat;
  WGPUFragmentState frag_state = WGPU_FRAGMENT_STATE_INIT;
  frag_state.module = shader_module.get();
  frag_state.entryPoint = gpu::to_string_view(kFragmentEntryPoint);
  frag_state.targetCount = 1;
  frag_state.targets = &color_target_state;
  render_pipeline_desc.fragment = &frag_state;

  // Discard the previous compile request now that we've received a new one
  discard_compile_request(runtime);

  pass_revision_ = pass.revision();
  common_revision_ = common_revision;

  current_compile_request_id_ =
    runtime.start_compile_request(render_pipeline_desc, shader_module, assembled_source.segments);
  compile_status_ = Pending{};
}

bool PassState::poll_compile_request(Runtime& runtime) {
  if (!current_compile_request_id_)
    return false;

  auto request_opt = runtime.take_compile_request(*current_compile_request_id_);

  if (!request_opt)
    return false;

  diagnostics_.clear();
  std::vector<CompileMessage>& messages = request_opt->messages.value();
  const SegmentMap& segments = request_opt->segments;

  for (CompileMessage& message : messages) {
    auto relative_line_opt = segments.find_relative_line(message.line_number);

    diagnostics_.push_back(
      Diagnostic{
        .severity = message.severity,
        .message = std::move(message.message),
        .location = relative_line_opt.transform([&message](const auto& relative_line) {
          return Diagnostic::Location{.line = relative_line, .column = message.line_column};
        }),
      }
    );
  }

  if (
    PipelineResult& pipeline_result = request_opt->pipeline_result.value();
    pipeline_result.has_value()
  ) {
    render_pipeline_ = std::move(pipeline_result.value());
    compile_status_ = Success{};
  } else {
    if (std::ranges::none_of(diagnostics_, [](const auto& diagnostic) {
          return diagnostic.severity == Severity::kError;
        })) {
      diagnostics_.push_back(
        Diagnostic{
          .severity = Severity::kError,
          .message = std::move(pipeline_result.error().message),
        }
      );
    }

    compile_status_ = Error{};
  }

  current_compile_request_id_.reset();
  return true;
}

void PassState::discard_compile_request(Runtime& runtime) {
  if (!current_compile_request_id_)
    return;

  runtime.discard_compile_request(current_compile_request_id_.value());
  current_compile_request_id_.reset();
}

bool PassState::is_stale(const Pass& pass, Revision common_revision) const {
  assert(pass_id_ == pass.id() && "PassState compared against a different Pass");

  return pass_revision_ != pass.revision() || common_revision_ != common_revision;
}

}  // namespace mewo::impl
