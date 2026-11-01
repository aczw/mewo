#include "impl/session.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"
#include "impl/session/blit_pass.hpp"
#include "impl/session/out_pass.hpp"

#include <webgpu/webgpu.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

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
  if (render_size_ == render_size)
    return;

  const gpu::Device& device = runtime_.device();

  render_size_ = render_size;
  out_pass_.view = create_out_tex(device, render_size);
  blit_pass_.bg = create_blit_bg(device, blit_pass_.bgl, out_pass_.view, blit_pass_.smp);
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
      elapsed_(0),
      out_pass_(std::move(out_pass)),
      blit_pass_(create_blit_pass(runtime.device(), out_pass_.view)) {}

void Session::render(impl::Surface* surface, uint64_t current_time) {
  const gpu::Queue& queue = runtime_.queue();
  const gpu::Device& device = runtime_.device();

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

  gpu::CmdEncoder cmd_encoder(wgpuDeviceCreateCommandEncoder(device.get(), &cmd_encoder_desc));

  {
    WGPURenderPassColorAttachment pass_color_attachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    pass_color_attachment.view = out_pass_.view.get();
    pass_color_attachment.loadOp = WGPULoadOp_Clear;
    pass_color_attachment.storeOp = WGPUStoreOp_Store;
    WGPURenderPassDescriptor pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    pass_desc.label = gpu::to_string_view("session-out-pass");
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &pass_color_attachment;

    gpu::RenderPassEncoder out_encoder(
      wgpuCommandEncoderBeginRenderPass(cmd_encoder.get(), &pass_desc)
    );

    wgpuRenderPassEncoderSetPipeline(out_encoder.get(), out_pass_.rpl.get());
    wgpuRenderPassEncoderSetBindGroup(out_encoder.get(), 0, out_pass_.bg.get(), 0, nullptr);
    wgpuRenderPassEncoderDraw(out_encoder.get(), 3, 0, 0, 0);
    wgpuRenderPassEncoderEnd(out_encoder.get());
  }

  std::optional<gpu::TexView> surface_view;

  if (surface)
    surface_view = surface->acquire(runtime_.adapter(), device);

  if (surface_view) {
    WGPUTextureFormat surface_format = surface->format();

    if (!blit_pass_.rpl || blit_pass_.surface_format != surface_format) {
      blit_pass_.rpl = create_blit_rpl(device, blit_pass_.bgl, surface_format);
      blit_pass_.surface_format = surface_format;
    }

    WGPURenderPassColorAttachment pass_color_attachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    pass_color_attachment.view = surface_view.value().get();
    pass_color_attachment.loadOp = WGPULoadOp_Clear;
    pass_color_attachment.storeOp = WGPUStoreOp_Store;
    WGPURenderPassDescriptor pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    pass_desc.label = gpu::to_string_view("session-blit-pass");
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &pass_color_attachment;

    gpu::RenderPassEncoder blit_encoder(
      wgpuCommandEncoderBeginRenderPass(cmd_encoder.get(), &pass_desc)
    );

    wgpuRenderPassEncoderSetPipeline(blit_encoder.get(), blit_pass_.rpl.get());
    wgpuRenderPassEncoderSetBindGroup(blit_encoder.get(), 0, blit_pass_.bg.get(), 0, nullptr);
    wgpuRenderPassEncoderDraw(blit_encoder.get(), 3, 0, 0, 0);
    wgpuRenderPassEncoderEnd(blit_encoder.get());
  }

  WGPUCommandBufferDescriptor cmd_buf_desc = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
  cmd_buf_desc.label = gpu::to_string_view("session-cmd-buf");

  gpu::CmdBuf cmd_buf(wgpuCommandEncoderFinish(cmd_encoder.get(), &cmd_buf_desc));
  WGPUCommandBuffer raw_cmd_buf = cmd_buf.get();
  wgpuQueueSubmit(queue.get(), 1, &raw_cmd_buf);

  if (surface_view)
    surface->present();
}

}  // namespace mewo::impl
