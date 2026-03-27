#include "mewo.hpp"

#include "editor.hpp"
#include "exception.hpp"
#include "gfx/create.hpp"
#include "gfx/frame_context.hpp"
#include "io.hpp"
#include "project.hpp"
#include "query.hpp"

#include <SDL3/SDL_timer.h>
#include <imgui_impl_sdl3.h>
#include <webgpu/webgpu_cpp.h>

#include <optional>
#include <print>

namespace mewo {

void Mewo::run() {
  SDL_Event event = {};

  const wgpu::Device& device = renderer_.device();
  const wgpu::Queue& queue = renderer_.queue();

  while (!pending_.quit()) {
    device.Tick();

    const gfx::FrameContext frame_ctx = prepare_new_frame();
    float current_time = static_cast<float>(SDL_GetTicksNS()) / 1'000'000'000.f;

    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      switch (event.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
          pending_.request_quit();
          break;
        }

        case SDL_EVENT_WINDOW_RESIZED: {
          auto [new_width, new_height] = window_.size_in_pixels();
          renderer_.resize(new_width, new_height);
          break;
        }
      }
    }

    layout_.build(pending_, window_, gui_ctx_, editor_, viewport_);

    viewport_.record(queue, frame_ctx, current_time);
    gui_ctx_.record(frame_ctx);

    static constexpr wgpu::CommandBufferDescriptor CMD_BUF_DESC = {.label = "command-buffer"};
    wgpu::CommandBuffer cmd_buf = frame_ctx.encoder.Finish(&CMD_BUF_DESC);

    queue.Submit(1, &cmd_buf);
    renderer_.surface().Present();
  }
}

const gfx::FrameContext Mewo::prepare_new_frame() {
  const gfx::FrameContext frame_ctx = renderer_.prepare_new_frame();

  if (auto& requested_open = pending_.project_open(); requested_open) {
    try {
      const auto& project = project_.emplace(requested_open.value());

      editor_.set_visible_code(io::read_wgsl_shader(project.shader_file_location()));
      pending_.request_run(editor_.combined_code());
      window_.update_project_in_title(project);
    } catch (const Exception& ex) {
      std::println("Failed to open project: {}", ex.what());
    }

    requested_open.reset();
  }

  // Since we're only making a copy of the current state, there's no need
  // to update the editor or request a new run.
  if (auto& requested_save_as = pending_.project_save_as(); requested_save_as) {
    try {
      project_ = Project::save_as(requested_save_as.value(), editor_.visible_code());
      window_.update_project_in_title(project_.value());
    } catch (const Exception& ex) {
      std::println("Project save as failed: {}", ex.what());
    }

    requested_save_as.reset();
  }

  if (bool& requested_save = pending_.project_save(); requested_save) {
    if (project_) {
      try {
        project_->save(editor_.visible_code());
        if constexpr (query::is_debug())
          std::println("Saved project \"{}\"", project_->name());
      } catch (const Exception& ex) {
        std::println("Project save failed: {}", ex.what());
      }
    } else {
      // TODO: a requested save with no currently active project should invoke a save as. This
      // requires first requesting the user for a directory to save in. That logic is currently
      // in `Layout::build`, so fix that first.
      std::println("No active project to save!");
    }

    requested_save = false;
  }

  if (auto& requested_resize = pending_.viewport_resize(); requested_resize) {
    auto [new_width, new_height] = requested_resize.value();

    if constexpr (query::is_debug())
      std::println("Viewport texture resized to {}×{}", new_width, new_height);

    viewport_.resize(renderer_.device(), new_width, new_height);
    requested_resize.reset();
  }

  if (auto& requested_run = pending_.run(); requested_run) {
    // TODO: check if the code is the same before creating new fragment shader module
    // (how expensive is this anyway?)
    auto compilation_result = gfx::create::shader_module_from_wgsl(
      renderer_, requested_run.value(), Viewport::FRAGMENT_SHADER_LABEL
    );

    editor_.set_diagnostics(std::move(compilation_result.second));

    if constexpr (query::is_debug())
      std::println("Shader compilation generated {} diagnostic(s)", editor_.diagnostics().size());

    if (const auto& fragment_module = compilation_result.first; fragment_module) {
      viewport_.update(fragment_module.value(), renderer_.device());

      if constexpr (query::is_debug())
        std::println("Updated viewport render pipeline");
    } else {
      if constexpr (query::is_debug())
        std::println("Shader compilation errors occurred, viewport render pipeline not updated");
    }

    requested_run.reset();
  }

  gui_ctx_.prepare_new_frame();

  return frame_ctx;
}

}  // namespace mewo
