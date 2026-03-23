#include "mewo.hpp"

#include "editor.hpp"
#include "exception.hpp"
#include "gfx/frame_context.hpp"
#include "io.hpp"
#include "project.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include <webgpu/webgpu_cpp.h>

#include <optional>
#include <print>

namespace mewo {

Mewo::Mewo()
    : renderer_(window_)
    , gui_ctx_(assets_, window_, renderer_)
    , editor_(assets_)
    , viewport_(assets_, state_, renderer_, editor_.combined_code())
{
}

void Mewo::run()
{
  SDL_Event event = {};

  const wgpu::Device& device = renderer_.device();
  const wgpu::Queue& queue = renderer_.queue();

  while (!pending_.quit) {
    device.Tick();

    const gfx::FrameContext frame_ctx = renderer_.prepare_new_frame();
    viewport_.prepare_new_frame(state_, renderer_);
    gui_ctx_.prepare_new_frame();

    if (state_.pending_project_open.has_value()) {
      try {
        const auto& project = project_.emplace(state_.pending_project_open.value());

        editor_.set_visible_code(
            io::read_wgsl_shader(project.root() / Project::WGSL_SHADER_FILE_NAME));
        viewport_.set_pending_run_request(editor_.combined_code());
        window_.set_project_in_title(project);
      } catch (const Exception& ex) {
        std::println("Failed to set new project: {}", ex.what());
      }

      state_.pending_project_open.reset();
    }

    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      switch (event.type) {
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
        pending_.quit = true;
        break;
      }

      case SDL_EVENT_WINDOW_RESIZED: {
        auto [new_width, new_height] = window_.size_in_pixels();
        renderer_.resize(new_width, new_height);
        break;
      }
      }
    }

    layout_.build(pending_, state_, window_, gui_ctx_, editor_, viewport_);

    viewport_.record(frame_ctx);
    gui_ctx_.record(frame_ctx);

    static constexpr wgpu::CommandBufferDescriptor CMD_BUF_DESC = { .label = "command-buffer" };
    wgpu::CommandBuffer cmd_buf = frame_ctx.encoder.Finish(&CMD_BUF_DESC);

    queue.Submit(1, &cmd_buf);
    renderer_.surface().Present();
  }
}

}
