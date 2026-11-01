#include "impl/session.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/session/out_pass.hpp"

#include <webgpu/webgpu.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

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

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Session>(
    new Session(runtime, project, render_size, create_out_pass(device, render_size))
  );
}

void Session::set_render_size(Dimensions render_size) {
  if (render_size_.width == render_size.width && render_size_.height == render_size.height)
    return;

  render_size_ = render_size;
  out_pass_.view = create_out_tex(runtime_.device(), render_size);
}

Session::Session(
  impl::Runtime& runtime,
  impl::Project& project,
  Dimensions render_size,
  OutPass&& out_pass
)
    : runtime_(runtime),
      project_(project),
      render_size_(render_size),
      previous_time_(std::nullopt),
      elapsed_(0),
      out_pass_(std::move(out_pass)) {}

void Session::render(impl::Surface* surface, uint64_t current_time) {
  const gpu::Queue& queue = runtime_.queue();
  uint64_t delta_time = 0;

  if (previous_time_ && current_time > *previous_time_)
    delta_time = current_time - *previous_time_;

  previous_time_ = current_time;
  elapsed_ += delta_time;

  OutPass::Uniforms out_unif = {
    .resolution =
      {
        static_cast<float>(render_size_.width),
        static_cast<float>(render_size_.height),
      },
    .time = ns_to_s(elapsed_),
    .delta_time = ns_to_s(delta_time),
  };
  wgpuQueueWriteBuffer(
    queue.get(), out_pass_.unif_buf.get(), 0, &out_unif, sizeof(OutPass::Uniforms)
  );

  WGPUCommandEncoderDescriptor cmd_encoder_desc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
  cmd_encoder_desc.label = gpu::to_string_view("session-cmd-encoder");

  gpu::CmdEncoder cmd_encoder(
    wgpuDeviceCreateCommandEncoder(runtime_.device().get(), &cmd_encoder_desc)
  );

  {
    WGPURenderPassColorAttachment out_pass_color_attachment =
      WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    out_pass_color_attachment.view = out_pass_.view.get();
    out_pass_color_attachment.loadOp = WGPULoadOp_Clear;
    out_pass_color_attachment.storeOp = WGPUStoreOp_Store;
    WGPURenderPassDescriptor out_pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    out_pass_desc.label = gpu::to_string_view("session-out-pass");
    out_pass_desc.colorAttachmentCount = 1;
    out_pass_desc.colorAttachments = &out_pass_color_attachment;

    gpu::RenderPassEncoder out_encoder(
      wgpuCommandEncoderBeginRenderPass(cmd_encoder.get(), &out_pass_desc)
    );

    wgpuRenderPassEncoderSetPipeline(out_encoder.get(), out_pass_.rpl.get());
    wgpuRenderPassEncoderSetBindGroup(out_encoder.get(), 0, out_pass_.bg.get(), 0, nullptr);
    wgpuRenderPassEncoderDraw(out_encoder.get(), 3, 0, 0, 0);
    wgpuRenderPassEncoderEnd(out_encoder.get());
  }

  WGPUCommandBufferDescriptor cmd_buf_desc = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
  cmd_buf_desc.label = gpu::to_string_view("session-cmd-buf");

  gpu::CmdBuf cmd_buf(wgpuCommandEncoderFinish(cmd_encoder.get(), &cmd_buf_desc));
  WGPUCommandBuffer raw_cmd_buf = cmd_buf.get();
  wgpuQueueSubmit(queue.get(), 1, &raw_cmd_buf);
}

}  // namespace mewo::impl
