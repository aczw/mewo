#include "mewo.hpp"

#include "editor.hpp"
#include "fs.hpp"
#include "gfx/frame_context.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo {

#if defined(MEWO_IS_DEBUG)
constexpr std::string_view VIEWPORT_FRAG_FILE_PATH = "../../assets/shaders/viewport.frag.wgsl";
#else
#error "TODO: handle "viewport.frag.wgsl" file path on release mode"
constexpr std::string_view VIEWPORT_FRAG_FILE_PATH = "viewport.frag.wgsl";
#endif

Mewo::Mewo()
    : sdl_ctx_()
    , window_()
    , renderer_(window_)
    , gui_ctx_(window_, renderer_)
    , editor_(fs::read_wgsl_shader(VIEWPORT_FRAG_FILE_PATH))
    , viewport_(renderer_, editor_.code())
{
}

void Mewo::run()
{
  SDL_Event event = {};

  const wgpu::Device& device = renderer_.device();

  while (!state_.should_quit) {
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      switch (event.type) {
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
        state_.should_quit = true;
        break;
      }

      case SDL_EVENT_WINDOW_RESIZED: {
        auto [new_width, new_height] = window_.size_in_pixels();
        renderer_.resize(new_width, new_height);
        break;
      }
      }
    }

    const gfx::FrameContext frame_ctx = renderer_.prepare_new_frame();
    gui_ctx_.prepare_new_frame();
    viewport_.apply_pending_resize(device);

    layout_.build(state_, gui_ctx_, device, editor_, viewport_);

    viewport_.record(frame_ctx);
    gui_ctx_.record(frame_ctx);

    static const wgpu::CommandBufferDescriptor CMD_BUF_DESC = { .label = "command-buffer" };
    wgpu::CommandBuffer cmd_buf = frame_ctx.encoder.Finish(&CMD_BUF_DESC);

    renderer_.queue().Submit(1, &cmd_buf);
    renderer_.surface().Present();
    device.Tick();
  }
}

}
