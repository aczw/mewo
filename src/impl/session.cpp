#include "impl/session.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "core/pass.hpp"
#include "core/shader/uniform_data.hpp"
#include "core/shader/uniforms.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/session/pass_state.hpp"
#include "impl/session/pass_targets.hpp"
#include "impl/session/surface_blit.hpp"

#include <webgpu/webgpu.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace mewo::impl {
namespace {

float ns_to_s(uint64_t time) { return static_cast<float>(static_cast<double>(time) * 1e-9); }

}  // namespace

std::optional<std::unique_ptr<Session>> Session::create(
  impl::Runtime& runtime,
  impl::Project& project,
  Dimensions render_size
) {
  const gpu::Device& device = runtime.device();

  // Device is not ready yet, cannot create a session
  if (!device)
    return std::nullopt;

  WGPUBufferDescriptor common_unifs_buf_desc = WGPU_BUFFER_DESCRIPTOR_INIT;
  common_unifs_buf_desc.label = gpu::to_string_view("common-unifs-buf");
  common_unifs_buf_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
  common_unifs_buf_desc.size = kUniformsLayout.size;

  gpu::Buf common_unifs_buf(wgpuDeviceCreateBuffer(device.get(), &common_unifs_buf_desc));

  WGPUBindGroupLayoutEntry common_unifs_bgl_entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
  common_unifs_bgl_entry.binding = 0;
  common_unifs_bgl_entry.visibility = WGPUShaderStage_Fragment;
  common_unifs_bgl_entry.buffer.type = WGPUBufferBindingType_Uniform;
  common_unifs_bgl_entry.buffer.minBindingSize = kUniformsLayout.size;

  WGPUBindGroupLayoutDescriptor common_unifs_bgl_desc = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
  common_unifs_bgl_desc.label = gpu::to_string_view("common-unifs-bgl");
  common_unifs_bgl_desc.entryCount = 1;
  common_unifs_bgl_desc.entries = &common_unifs_bgl_entry;

  gpu::BindGroupLayout common_unifs_bgl(
    wgpuDeviceCreateBindGroupLayout(device.get(), &common_unifs_bgl_desc)
  );

  WGPUBindGroupEntry common_unifs_bg_entry = WGPU_BIND_GROUP_ENTRY_INIT;
  common_unifs_bg_entry.binding = 0;
  common_unifs_bg_entry.buffer = common_unifs_buf.get();
  common_unifs_bg_entry.size = kUniformsLayout.size;

  WGPUBindGroupDescriptor common_unifs_bg_desc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
  common_unifs_bg_desc.label = gpu::to_string_view("common-unifs-bg");
  common_unifs_bg_desc.layout = common_unifs_bgl.get();
  common_unifs_bg_desc.entryCount = 1;
  common_unifs_bg_desc.entries = &common_unifs_bg_entry;

  gpu::BindGroup common_unifs_bg(wgpuDeviceCreateBindGroup(device.get(), &common_unifs_bg_desc));

  PassTargets targets(device, render_size);

  std::vector<PassState> states;
  states.reserve(project.passes().size());
  for (const Pass& pass : project.passes()) {
    PassState& inserted_state =
      states.emplace_back(device, common_unifs_bgl, targets.bind_group_layout(), pass);
    inserted_state.request_render_pipeline_build(runtime, pass);
  }

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Session>(new Session(
    runtime,
    project,
    render_size,
    std::move(common_unifs_buf),
    std::move(common_unifs_bgl),
    std::move(common_unifs_bg),
    std::move(states),
    std::move(targets)
  ));
}

Session::~Session() {
  for (PassState& state : states_)
    state.discard_compile_request(runtime_);
}

void Session::set_render_size(Dimensions render_size) {
  if (render_size_ == render_size)
    return;

  const gpu::Device& device = runtime_.device();

  render_size_ = render_size;
  targets_.resize(device, render_size);
}

Session::Session(
  impl::Runtime& runtime,
  impl::Project& project,
  Dimensions render_size,
  gpu::Buf&& common_unifs_buf,
  gpu::BindGroupLayout&& common_unifs_bgl,
  gpu::BindGroup&& common_unifs_bg,
  std::vector<PassState>&& states,
  PassTargets&& targets
)
    : runtime_(runtime),
      project_(project),
      render_size_(render_size),
      elapsed_(0),
      frame_number_(0),
      common_unifs_buf_(std::move(common_unifs_buf)),
      common_unifs_bgl_(std::move(common_unifs_bgl)),
      common_unifs_bg_(std::move(common_unifs_bg)),
      states_(std::move(states)),
      targets_(std::move(targets)),
      status_revision_(0) {}

PassState* Session::get_state(Pass::Id pass_id) {
  if (auto state_it = std::ranges::find(states_, pass_id, &PassState::pass_id);
      state_it != states_.end()) {
    return std::to_address(state_it);
  }

  return nullptr;
}

void Session::render(impl::Surface* surface, uint64_t current_time) {
  const gpu::Queue& queue = runtime_.queue();
  const gpu::Device& device = runtime_.device();

  uint64_t delta_time = 0;

  if (previous_time_ && current_time > *previous_time_)
    delta_time = current_time - *previous_time_;

  previous_time_ = current_time;
  elapsed_ += delta_time;

  UniformData common;
  common.set(
    UniformMember::kResolution,
    {static_cast<float>(render_size_.width), static_cast<float>(render_size_.height)}
  );
  common.set(UniformMember::kTime, ns_to_s(elapsed_));
  common.set(UniformMember::kDeltaTime, ns_to_s(delta_time));
  common.set(UniformMember::kFrameNumber, frame_number_);
  common.set(UniformMember::kFrameRate, [&delta_time]() -> uint32_t {
    if (delta_time == 0)
      return 0;
    return static_cast<uint32_t>(std::round(1.f / ns_to_s(delta_time)));
  }());

  wgpuQueueWriteBuffer(
    queue.get(), common_unifs_buf_.get(), 0, common.bytes().data(), common.bytes().size()
  );

  for (PassState& state : states_) {
    if (state.poll_compile_request(runtime_))
      status_revision_ += 1;
  }

  WGPUCommandEncoderDescriptor cmd_encoder_desc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
  cmd_encoder_desc.label = gpu::to_string_view("session-cmd-encoder");

  gpu::CmdEncoder cmd_encoder(wgpuDeviceCreateCommandEncoder(device.get(), &cmd_encoder_desc));

  targets_.record_clear_pass(cmd_encoder);

  const std::vector<Pass>& passes = project_.passes();

  for (size_t idx = 0; idx < passes.size(); ++idx) {
    const Pass& pass = passes[idx];
    PassState* state = get_state(pass.id());
    assert(state);

    if (state->is_stale(pass)) {
      state->request_render_pipeline_build(runtime_, pass);
      status_revision_ += 1;
    }

    WGPURenderPassColorAttachment pass_color_attachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    pass_color_attachment.view = targets_.target_view(idx).get();
    pass_color_attachment.loadOp = WGPULoadOp_Clear;
    pass_color_attachment.storeOp = WGPUStoreOp_Store;
    WGPURenderPassDescriptor pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    pass_desc.label = gpu::to_string_view(state->pass_label());
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &pass_color_attachment;

    gpu::RenderPassEncoder pass_encoder(
      wgpuCommandEncoderBeginRenderPass(cmd_encoder.get(), &pass_desc)
    );

    // Pipeline may not be ready yet because it's compiling asynchronously
    if (const auto& render_pipeline = state->render_pipeline(); render_pipeline) {
      wgpuRenderPassEncoderSetPipeline(pass_encoder.get(), render_pipeline.get());
      wgpuRenderPassEncoderSetBindGroup(pass_encoder.get(), 0, common_unifs_bg_.get(), 0, nullptr);
      wgpuRenderPassEncoderSetBindGroup(
        pass_encoder.get(), 1, targets_.prev_bind_group(idx).get(), 0, nullptr
      );
      wgpuRenderPassEncoderDraw(pass_encoder.get(), 3, 1, 0, 0);
    }

    wgpuRenderPassEncoderEnd(pass_encoder.get());
  }

  std::optional<gpu::TexView> surface_view;

  if (surface)
    surface_view = surface->acquire(runtime_.adapter(), device);

  if (surface_view) {
    WGPUTextureFormat surface_format = surface->format();

    blit_.prepare(device, targets_.bind_group_layout(), surface_format);

    WGPURenderPassColorAttachment pass_color_attachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    pass_color_attachment.view = surface_view.value().get();
    pass_color_attachment.loadOp = WGPULoadOp_Clear;
    pass_color_attachment.storeOp = WGPUStoreOp_Store;
    WGPURenderPassDescriptor pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    pass_desc.label = gpu::to_string_view("surface-blit");
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &pass_color_attachment;

    gpu::RenderPassEncoder blit_encoder(
      wgpuCommandEncoderBeginRenderPass(cmd_encoder.get(), &pass_desc)
    );

    wgpuRenderPassEncoderSetPipeline(blit_encoder.get(), blit_.render_pipeline().get());
    wgpuRenderPassEncoderSetBindGroup(
      blit_encoder.get(), 0, targets_.prev_bind_group(passes.size()).get(), 0, nullptr
    );
    wgpuRenderPassEncoderDraw(blit_encoder.get(), 3, 1, 0, 0);
    wgpuRenderPassEncoderEnd(blit_encoder.get());
  }

  WGPUCommandBufferDescriptor cmd_buf_desc = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
  cmd_buf_desc.label = gpu::to_string_view("session-cmd-buf");

  gpu::CmdBuf cmd_buf(wgpuCommandEncoderFinish(cmd_encoder.get(), &cmd_buf_desc));
  WGPUCommandBuffer raw_cmd_buf = cmd_buf.get();
  wgpuQueueSubmit(queue.get(), 1, &raw_cmd_buf);

  if (surface_view)
    surface->present();

  frame_number_ += 1;
}

}  // namespace mewo::impl
