#include "mewo.hpp"

#include "gfx/frame_context.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include <webgpu/webgpu_cpp.h>

namespace mewo {

Mewo::Mewo()
    : sdl_ctx_()
    , window_()
    , renderer_(window_)
    , gui_(window_, renderer_)
{
}

void Mewo::run()
{
  bool will_quit = false;
  SDL_Event event = {};

  while (!will_quit) {
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      if (event.type == SDL_EVENT_QUIT
          || (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED
              && event.window.windowID == SDL_GetWindowID(window_.get()))) {
        will_quit = true;
      }
    }

    const gfx::FrameContext frame_ctx = renderer_.prepare_new_frame();

    gui_.record(frame_ctx);

    static const wgpu::CommandBufferDescriptor CMD_BUF_DESC = { .label = "command-buffer" };
    wgpu::CommandBuffer cmd_buf = frame_ctx.encoder.Finish(&CMD_BUF_DESC);

    renderer_.queue().Submit(1, &cmd_buf);
    renderer_.surface().Present();
    renderer_.device().Tick();
  }
}

}
