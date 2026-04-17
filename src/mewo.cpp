#include "mewo.hpp"

#include "event/event.hpp"
#include "exception.hpp"
#include "gfx/create.hpp"
#include "gfx/frame_context.hpp"
#include "gui/editor/editor.hpp"
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

#include <cstdint>
#include <filesystem>
#include <functional>
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

std::optional<const char*> parse_dir_from_filelist(const char* const* filelist) {
  if (filelist == nullptr) {
    std::println("Error while showing folder dialog: {}", SDL_GetError());
    return {};
  }

  if (*filelist == nullptr)
    return {};

  const char* dir_path = nullptr;
  int count = 0;

  while (*filelist) {
    dir_path = *filelist;
    filelist += 1;
    count += 1;
  }

  if (count > 1)
    std::println("warning: more than one folder selected, using last one");

  return dir_path;
}

}  // namespace

Mewo::Mewo()
    : executable_dir_(os::find_executable_dir()),
      assets_dir_(find_assets_dir(executable_dir_)),
      gfx_(event_queue_, window_),
      gui_(assets_dir_, window_, gfx_),
      editor_(event_queue_, assets_dir_, gui_.theme()),
      viewport_(event_queue_, assets_dir_, gfx_, editor_.combined_code()),
      current_time_(SDL_GetTicks()) {}

void Mewo::run() {
  while (!should_quit_) {
    process_queued_events();

    const gfx::FrameContext frame_ctx = gfx_.begin_frame();
    gui_.begin_frame();

    uint64_t now = SDL_GetTicks();
    update(frame_ctx, now - current_time_);
    current_time_ = now;
  }
}

void Mewo::process_queued_events() {
  for (const auto& event : event_queue_.drain()) {
    std::visit(
      util::Match{
        [this](const QuitRequest&) { should_quit_ = true; },

        [this](const ChooseFolderRequest& cfr) {
          auto callback_fn = std::invoke([reason = cfr.reason]() -> SDL_DialogFileCallback {
            switch (reason) {
              case ChooseFolderRequest::Reason::ProjectOpen:
                return [](void* userdata, const char* const* filelist, int) -> void {
                  if (auto dir_opt = parse_dir_from_filelist(filelist); dir_opt) {
                    static_cast<Mewo*>(userdata)->event_queue_.push(
                      ProjectOpenRequest{.directory = dir_opt.value()}
                    );
                  }
                };

              case ChooseFolderRequest::Reason::ProjectSaveAs:
                return [](void* userdata, const char* const* filelist, int) -> void {
                  if (auto dir_opt = parse_dir_from_filelist(filelist); dir_opt) {
                    static_cast<Mewo*>(userdata)->event_queue_.push(
                      ProjectSaveAsRequest{.directory = dir_opt.value()}
                    );
                  }
                };

              default: util::enum_unreachable("event::ChooseFolderRequest::Reason", reason);
            }
          });

          // TODO: probably can't use this for macOS because it doesn't let you create a
          // folder from within the dialog by default
          SDL_ShowOpenFolderDialog(callback_fn, this, window_.get(), nullptr, false);
        },

        [this](const ProjectOpenRequest& open_req) {
          try {
            const auto& project = project_.emplace(open_req.directory);

            editor_.set_visible_code(io::read_wgsl_shader(project.shader_file_location()));
            event_queue_.push(RunRequest{.fragment_code = editor_.combined_code()});
            window_.update_project_in_title(project);
          } catch (const Exception& ex) {
            std::println("Failed to open project: {}", ex.what());
          }
        },

        // Since we're only making a copy of the current state, there's no need
        // to update the editor or request a new run.
        [this](const ProjectSaveAsRequest& save_as_req) {
          try {
            project_ = Project::save_as(save_as_req.directory, editor_);
            window_.update_project_in_title(project_.value());
          } catch (const Exception& ex) {
            std::println("Project save as failed: {}", ex.what());
          }
        },

        [this](const ProjectSaveRequest&) {
          if (project_) {
            try {
              project_->save(editor_);
              if constexpr (query::is_debug())
                std::println("Saved project \"{}\"", project_->name());
            } catch (const Exception& ex) {
              std::println("Project save failed: {}", ex.what());
            }
          } else {
            using CFR = ChooseFolderRequest;
            event_queue_.push(CFR{.reason = CFR::Reason::ProjectSaveAs});
          }
        },

        [this](const ViewportResizeRequest& resize_req) {
          auto [new_width, new_height] = resize_req;

          if constexpr (query::is_debug())
            std::println("Viewport texture resized to {}×{}", new_width, new_height);

          viewport_.resize(gfx_.device(), new_width, new_height);
        },

        [this](const RunRequest& run_req) {
          // TODO: check if the code is the same before creating new fragment shader module
          // (how expensive is this anyway?)
          auto compilation_result = gfx::create::shader_module_from_wgsl(
            gfx_, run_req.fragment_code, Viewport::FRAGMENT_SHADER_LABEL
          );

          editor_.set_diagnostics(std::move(compilation_result.second));

          if constexpr (query::is_debug())
            std::println(
              "Shader compilation generated {} diagnostic(s)", editor_.diagnostics().size()
            );

          if (const auto& fragment_module = compilation_result.first; fragment_module) {
            viewport_.rebuild_render_pipeline(fragment_module.value(), gfx_.device());

            if constexpr (query::is_debug())
              std::println("Updated viewport render pipeline");
          } else {
            if constexpr (query::is_debug())
              std::println(
                "Shader compilation errors occurred, viewport render pipeline not updated"
              );
          }
        },

        [this](const WindowResized& resize) {
          auto [new_width, new_height] = resize;
          gfx_.resize_surface(new_width, new_height);
        },

        [](const WGPUDeviceLost& error) {
          throw Exception(
            "WebGPU device lost. Reason: {}. Message (below):\n{}", error.reason, error.message
          );
        },

        [](const WGPUUncapturedError& error) {
          std::println(
            "Uncaptured WebGPU error. Type: {}. Message (below):\n{}",
            error.type_name,
            error.message
          );
        },

        [this](const EditorTextChanged) {
          if (auto_compiler_.is_running()) {
            auto_compiler_.reset();
          } else {
            auto_compiler_.start();
          }
        },

        [this](const AutoCompilerElapsed) {
          event_queue_.push(RunRequest{.fragment_code = editor_.combined_code()});
        },

        [](auto&&) { std::unreachable(); },
      },
      event
    );
  }  // namespace mewo
}

void Mewo::update(const gfx::FrameContext& frame_ctx, uint64_t delta_time) {
  SDL_Event event = {};

  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);

    switch (event.type) {
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
        event_queue_.push(QuitRequest{});
        break;
      }

      case SDL_EVENT_WINDOW_RESIZED: {
        auto [new_width, new_height] = window_.size_in_pixels();
        event_queue_.push(WindowResized{.new_width = new_width, .new_height = new_height});
        break;
      }
    }
  }

  gui_.build_layout(event_queue_, frame_ctx, editor_, viewport_);

  auto_compiler_.update(event_queue_, delta_time);
  viewport_.update(gfx_.queue(), frame_ctx, static_cast<float>(current_time_) * 1e-3f);
  gui_.update(frame_ctx);

  static constexpr wgpu::CommandBufferDescriptor CMD_BUF_DESC = {.label = "command-buffer"};
  gfx_.update(frame_ctx.encoder.Finish(&CMD_BUF_DESC));
}

}  // namespace mewo
