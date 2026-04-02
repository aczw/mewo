#include "mewo.hpp"

#include "editor.hpp"
#include "event/event.hpp"
#include "exception.hpp"
#include "gfx/create.hpp"
#include "gfx/frame_context.hpp"
#include "io.hpp"
#include "os.hpp"
#include "project.hpp"
#include "query.hpp"
#include "util/enum_unreachable.hpp"
#include "util/match.hpp"

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_timer.h>
#include <imgui_impl_sdl3.h>
#include <webgpu/webgpu_cpp.h>

#include <filesystem>
#include <optional>
#include <print>
#include <utility>
#include <variant>

namespace mewo {

namespace {

std::filesystem::path find_assets_dir(const std::filesystem::path& executable_dir) {
  using namespace std::string_view_literals;

  // There are two possible places where assets can be located, depending on whether
  // the executable is launched from the build directory (during development), or from
  // the final release folder. This way I can support both.
  static constexpr std::array POSSIBLE_ASSETS_DIRS = {
    "./assets"sv,
    "../../assets"sv,
  };

  std::filesystem::path assets_dir;

  for (auto possible_dir : POSSIBLE_ASSETS_DIRS) {
    if (auto full_path = executable_dir / possible_dir; std::filesystem::exists(full_path)) {
      assets_dir = std::filesystem::canonical(full_path);
      break;
    }
  }

  if (assets_dir.empty()) [[unlikely]] {
    throw Exception("Assets directory was not found");
  } else {
    std::println("Assets directory: {}", assets_dir.string());
  }

  return assets_dir;
}

}  // namespace

Mewo::Mewo()
    : executable_dir_(os::find_executable_dir()),
      assets_dir_(find_assets_dir(executable_dir_)),
      gfx_(window_),
      gui_(assets_dir_, window_, gfx_),
      editor_(assets_dir_),
      viewport_(pending_, assets_dir_, gfx_, editor_.combined_code()) {}

void Mewo::run() {
  while (!should_quit_) {
    process_queued_events();
    apply_pending_actions();

    const gfx::FrameContext frame_ctx = gfx_.begin_frame();
    gui_.begin_frame();

    update(frame_ctx);
  }
}

void Mewo::process_queued_events() {
  for (const auto& event : event_queue_.drain()) {
    using namespace event;

    std::visit(
      util::Match{
        [this](const QuitRequest&) { should_quit_ = true; },

        [this](const ChooseFolderRequest& cfr) {
          using CFR = ChooseFolderRequest;

          switch (cfr.reason) {
            case CFR::Reason::ProjectOpen: {
              show_open_folder_dialog<CFR::Reason::ProjectOpen>();
              break;
            }

            case CFR::Reason::ProjectSaveAs: {
              show_open_folder_dialog<CFR::Reason::ProjectSaveAs>();
              break;
            }

            default: util::enum_unreachable("event::ChooseFolderRequest::Reason", cfr.reason);
          }
        },

        [this](const ProjectOpenRequest& open_req) {
          try {
            const auto& project = project_.emplace(open_req.directory);

            editor_.set_visible_code(io::read_wgsl_shader(project.shader_file_location()));
            pending_.request_run(editor_.combined_code());
            window_.update_project_in_title(project);
          } catch (const Exception& ex) {
            std::println("Failed to open project: {}", ex.what());
          }
        },

        // Since we're only making a copy of the current state, there's no need
        // to update the editor or request a new run.
        [this](const ProjectSaveAsRequest& save_as_req) {
          try {
            project_ = Project::save_as(save_as_req.directory, editor_.visible_code());
            window_.update_project_in_title(project_.value());
          } catch (const Exception& ex) {
            std::println("Project save as failed: {}", ex.what());
          }
        },

        [](const auto&) { std::unreachable(); },
      },
      event
    );
  }
}

void Mewo::apply_pending_actions() {
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

    viewport_.resize(gfx_.device(), new_width, new_height);
    requested_resize.reset();
  }

  if (auto& requested_run = pending_.run(); requested_run) {
    // TODO: check if the code is the same before creating new fragment shader module
    // (how expensive is this anyway?)
    auto compilation_result = gfx::create::shader_module_from_wgsl(
      gfx_, requested_run.value(), Viewport::FRAGMENT_SHADER_LABEL
    );

    editor_.set_diagnostics(std::move(compilation_result.second));

    if constexpr (query::is_debug())
      std::println("Shader compilation generated {} diagnostic(s)", editor_.diagnostics().size());

    if (const auto& fragment_module = compilation_result.first; fragment_module) {
      viewport_.update(fragment_module.value(), gfx_.device());

      if constexpr (query::is_debug())
        std::println("Updated viewport render pipeline");
    } else {
      if constexpr (query::is_debug())
        std::println("Shader compilation errors occurred, viewport render pipeline not updated");
    }

    requested_run.reset();
  }
}

void Mewo::update(const gfx::FrameContext& frame_ctx) {
  SDL_Event event = {};

  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);

    switch (event.type) {
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
        event_queue_.push(event::QuitRequest{});
        break;
      }

      case SDL_EVENT_WINDOW_RESIZED: {
        auto [new_width, new_height] = window_.size_in_pixels();
        gfx_.resize_surface(new_width, new_height);
        break;
      }
    }
  }

  gui_.build_layout(event_queue_, pending_, editor_, viewport_);

  float current_time = static_cast<float>(SDL_GetTicks()) * 1e-3f;

  viewport_.record(gfx_.queue(), frame_ctx, current_time);
  gui_.record(frame_ctx);

  static constexpr wgpu::CommandBufferDescriptor CMD_BUF_DESC = {.label = "command-buffer"};
  gfx_.update(frame_ctx.encoder.Finish(&CMD_BUF_DESC));
}

}  // namespace mewo
