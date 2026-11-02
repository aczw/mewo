#include "impl/session/pass_targets.hpp"

#include "core/gpu/string.hpp"
#include "core/gpu/types.hpp"
#include "impl/common/dimensions.hpp"

#include <array>
#include <string_view>

namespace mewo::impl {
namespace {

std::array<gpu::TexView, 2> create_views(const gpu::Device& device, Dimensions render_size) {
  std::array<gpu::Tex, 2> textures = {};

  for (size_t idx = 0; idx < 2; ++idx) {
    WGPUTextureDescriptor tex_desc = WGPU_TEXTURE_DESCRIPTOR_INIT;
    tex_desc.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
    tex_desc.format = PassTargets::kTexFormat;
    tex_desc.size.width = render_size.width;
    tex_desc.size.height = render_size.height;

    textures[idx] = gpu::Tex(wgpuDeviceCreateTexture(device.get(), &tex_desc));
  }

  WGPUTextureViewDescriptor view_desc_a = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
  view_desc_a.label = gpu::to_string_view("pass-targets-view-a");
  WGPUTextureViewDescriptor view_desc_b = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
  view_desc_b.label = gpu::to_string_view("pass-targets-view-b");

  return {
    gpu::TexView(wgpuTextureCreateView(textures[0].get(), &view_desc_a)),
    gpu::TexView(wgpuTextureCreateView(textures[1].get(), &view_desc_b))
  };
}

gpu::BindGroup create_bg(
  const gpu::Device& device,
  const gpu::BindGroupLayout& bgl,
  const gpu::TexView& source,
  const gpu::Sampler& smp,
  std::string_view label
) {
  std::array<WGPUBindGroupEntry, 2> bg_entries = {
    WGPU_BIND_GROUP_ENTRY_INIT,
    WGPU_BIND_GROUP_ENTRY_INIT,
  };

  bg_entries[0].binding = 0;
  bg_entries[0].textureView = source.get();

  bg_entries[1].binding = 1;
  bg_entries[1].sampler = smp.get();

  WGPUBindGroupDescriptor bg_desc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
  bg_desc.label = gpu::to_string_view(label);
  bg_desc.layout = bgl.get();
  bg_desc.entryCount = bg_entries.size();
  bg_desc.entries = bg_entries.data();

  return gpu::BindGroup(wgpuDeviceCreateBindGroup(device.get(), &bg_desc));
}

}  // namespace

PassTargets::PassTargets(const gpu::Device& device, Dimensions render_size) {
  views_ = create_views(device, render_size);

  WGPUSamplerDescriptor sampler_desc = WGPU_SAMPLER_DESCRIPTOR_INIT;
  sampler_desc.label = gpu::to_string_view("pass-targets-smp");
  sampler_desc.magFilter = WGPUFilterMode_Linear;
  sampler_desc.minFilter = WGPUFilterMode_Linear;

  sampler_ = gpu::Sampler(wgpuDeviceCreateSampler(device.get(), &sampler_desc));

  std::array<WGPUBindGroupLayoutEntry, 2> bgl_entries = {
    WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
    WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
  };

  bgl_entries[0].binding = 0;
  bgl_entries[0].visibility = WGPUShaderStage_Fragment;
  bgl_entries[0].texture.sampleType = WGPUTextureSampleType_Float;
  bgl_entries[0].texture.viewDimension = WGPUTextureViewDimension_2D;

  bgl_entries[1].binding = 1;
  bgl_entries[1].visibility = WGPUShaderStage_Fragment;
  bgl_entries[1].sampler.type = WGPUSamplerBindingType_Filtering;

  WGPUBindGroupLayoutDescriptor bgl_desc = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
  bgl_desc.label = gpu::to_string_view("pass-targets-bgl");
  bgl_desc.entryCount = bgl_entries.size();
  bgl_desc.entries = bgl_entries.data();

  bind_group_layout_ =
    gpu::BindGroupLayout(wgpuDeviceCreateBindGroupLayout(device.get(), &bgl_desc));

  bind_groups_ = {
    create_bg(device, bind_group_layout_, views_[0], sampler_, "pass-targets-bg-a"),
    create_bg(device, bind_group_layout_, views_[1], sampler_, "pass-targets-bg-b")
  };
}

void PassTargets::resize(const gpu::Device& device, Dimensions new_render_size) {
  views_ = create_views(device, new_render_size);

  bind_groups_ = {
    create_bg(device, bind_group_layout_, views_[0], sampler_, "pass-targets-bg-a"),
    create_bg(device, bind_group_layout_, views_[1], sampler_, "pass-targets-bg-b")
  };
}

void PassTargets::clear(const gpu::CmdEncoder& cmd_encoder) const {
  WGPURenderPassColorAttachment color_attachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
  color_attachment.view = views_[1].get();
  color_attachment.loadOp = WGPULoadOp_Clear;
  color_attachment.storeOp = WGPUStoreOp_Store;
  color_attachment.clearValue = {.r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0};

  WGPURenderPassDescriptor pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
  pass_desc.label = gpu::to_string_view("pass-targets-clear");
  pass_desc.colorAttachmentCount = 1;
  pass_desc.colorAttachments = &color_attachment;

  gpu::RenderPassEncoder clear_encoder(
    wgpuCommandEncoderBeginRenderPass(cmd_encoder.get(), &pass_desc)
  );

  wgpuRenderPassEncoderEnd(clear_encoder.get());
}

}  // namespace mewo::impl
