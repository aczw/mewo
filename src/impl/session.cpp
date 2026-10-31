#include "impl/session.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"

#include <webgpu/webgpu.h>

#include <cstdint>
#include <memory>
#include <optional>

namespace mewo::impl {

std::optional<std::unique_ptr<Session>> Session::create(
  impl::Runtime& runtime,
  impl::Project& project,
  Dimensions render_size
) {
  const gpu::Device& device = runtime.device();

  // Device is not ready yet, cannot create a session
  if (!device)
    return std::nullopt;

  WGPUBufferDescriptor unif_buf_desc = WGPU_BUFFER_DESCRIPTOR_INIT;
  unif_buf_desc.label = gpu::to_string_view("session-unif-buf");
  unif_buf_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
  unif_buf_desc.size = sizeof(Uniforms);
  gpu::Buf unif_buf(wgpuDeviceCreateBuffer(device.get(), &unif_buf_desc));

  WGPUBindGroupLayoutEntry rp_unif_bgl_entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
  rp_unif_bgl_entry.binding = 0;
  rp_unif_bgl_entry.visibility = WGPUShaderStage_Fragment;
  rp_unif_bgl_entry.buffer.type = WGPUBufferBindingType_Uniform;
  rp_unif_bgl_entry.buffer.minBindingSize = sizeof(Uniforms);

  WGPUBindGroupLayoutDescriptor rp_bgl_desc = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
  rp_bgl_desc.label = gpu::to_string_view("session-rp-bgl");
  rp_bgl_desc.entryCount = 1;
  rp_bgl_desc.entries = &rp_unif_bgl_entry;

  gpu::BindGroupLayout rp_bgl(wgpuDeviceCreateBindGroupLayout(device.get(), &rp_bgl_desc));
  WGPUBindGroupLayout raw_rp_bgl = rp_bgl.get();

  WGPUBindGroupEntry rp_unif_bg_entry = WGPU_BIND_GROUP_ENTRY_INIT;
  rp_unif_bg_entry.binding = 0;
  rp_unif_bg_entry.buffer = unif_buf.get();
  rp_unif_bg_entry.size = sizeof(Uniforms);

  WGPUBindGroupDescriptor rp_bg_desc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
  rp_bg_desc.label = gpu::to_string_view("session-rp-bg");
  rp_bg_desc.layout = rp_bgl.get();
  rp_bg_desc.entryCount = 1;
  rp_bg_desc.entries = &rp_unif_bg_entry;

  gpu::BindGroup rp_bg(wgpuDeviceCreateBindGroup(device.get(), &rp_bg_desc));

  WGPUPipelineLayoutDescriptor pl_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
  pl_desc.label = gpu::to_string_view("session-pl");
  pl_desc.bindGroupLayoutCount = 1;
  pl_desc.bindGroupLayouts = &raw_rp_bgl;

  WGPURenderPipelineDescriptor rpl_desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
  rpl_desc.label = gpu::to_string_view("session-rpl");
  rpl_desc.layout = wgpuDeviceCreatePipelineLayout(device.get(), &pl_desc);

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Session>(new Session(runtime, project, render_size));
}

Session::Session(impl::Runtime& runtime, impl::Project& project, Dimensions render_size)
    : runtime_(runtime),
      project_(project),
      render_size_(render_size),
      previous_time_(std::nullopt),
      elapsed_(0) {}

void Session::render(impl::Surface* surface, uint64_t current_time) {
  uint64_t delta_time = 0;

  if (previous_time_ && current_time > *previous_time_)
    delta_time = current_time - *previous_time_;

  previous_time_ = current_time;
  elapsed_ += delta_time;
}

}  // namespace mewo::impl
