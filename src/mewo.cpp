#include "mewo.hpp"

#include "event/event.hpp"
#include "exception.hpp"
#include "gfx/create.hpp"
#include "gfx/frame_context.hpp"
#include "gui/editor/auto_compiler.hpp"
#include "gui/editor/editor.hpp"
#include "io.hpp"
#include "os.hpp"
#include "project.hpp"
#include "query.hpp"
#include "util/enum_unreachable.hpp"
#include "util/match.hpp"

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
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
      auto_compiler_(AutoCompiler()),
      previous_time_(SDL_GetTicks()) {}

void Mewo::run() {
  while (!should_quit_) {
    process_queued_events();

    const gfx::FrameContext frame_ctx = gfx_.begin_frame();
    gui_.begin_frame();

    uint64_t now = SDL_GetTicks();
    update(frame_ctx, now - previous_time_);
    previous_time_ = now;

    gfx_.end_frame(std::move(frame_ctx));
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

              default: util::enum_unreachable("ChooseFolderRequest::Reason", reason);
            }
          });

          // TODO: probably shouldn't use this for macOS because it doesn't let you create a
          // folder from within the dialog by default
          SDL_ShowOpenFolderDialog(callback_fn, this, window_.get(), nullptr, false);
        },

        [this](const ProjectOpenRequest& open_req) {
          try {
            const auto& project = project_.emplace(open_req.directory);

            editor_.set_visible_code(io::read_wgsl_shader(project.shader_file_location()));
            event_queue_.push(RunRequest{.fragment_code = editor_.combined_code()});
            window_.update_project_in_title(project);

            if (!viewport_.is_playing())
              viewport_.toggle_playback();
            viewport_.reset_playback_time();
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
              window_.update_dirty_status_in_title(false);
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

        [this](const ViewportPlaybackToggled) { viewport_.toggle_playback(); },

        [this](const ViewportPlaybackTimeResetRequest) { viewport_.reset_playback_time(); },

        [this](const RunRequest& run_req) {
          // TODO: check if the code is the same before creating new fragment shader module
          // (how expensive is this anyway?)
          auto result = gfx::create::shader_module_from_wgsl(
            gfx_, run_req.fragment_code, Viewport::FRAGMENT_SHADER_LABEL
          );

          editor_.set_diagnostics(std::move(result.diagnostics));
          gui_.set_last_compilation_duration(result.time_elapsed);

          if (const auto& fragment_module = result.shader_module; fragment_module) {
            viewport_.rebuild_render_pipeline(fragment_module.value(), gfx_.device());
            gui_.set_is_last_compilation_successful(true);

            if constexpr (query::is_debug())
              std::println("Updated viewport render pipeline");
          } else {
            gui_.set_is_last_compilation_successful(false);

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
          if (auto_compiler_) {
            if (auto_compiler_->is_running()) {
              auto_compiler_->reset();
            } else {
              auto_compiler_->start();
            }
          }

          if (project_)
            window_.update_dirty_status_in_title(editor_.is_dirty());
        },

        [this](const AutoCompilerElapsed) {
          event_queue_.push(RunRequest{.fragment_code = editor_.combined_code()});
        },

        [this](const HotReloadingToggled) {
          if (auto_compiler_) {
            auto_compiler_.reset();
          } else {
            auto_compiler_ = AutoCompiler();
          }
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

      case SDL_EVENT_KEY_DOWN: handle_keyboard_event(event.key); break;
    }
  }

  gui_.build_layout(event_queue_, frame_ctx, editor_, viewport_, auto_compiler_);

  if (auto_compiler_)
    auto_compiler_->update(event_queue_, delta_time);

  static constexpr float MILLISECONDS_TO_SECONDS = 1e-3f;
  viewport_.update(
    gfx_.queue(), frame_ctx, static_cast<float>(delta_time) * MILLISECONDS_TO_SECONDS
  );

  gui_.update(frame_ctx);
}

void Mewo::handle_keyboard_event(const SDL_KeyboardEvent& kbd_event) {
  using CFR = ChooseFolderRequest;

  const bool has_primary = (kbd_event.mod & os::PRIMARY_MOD_KEY) != 0;
  const bool has_shift = (kbd_event.mod & SDL_KMOD_SHIFT) != 0;
  const bool has_alt = (kbd_event.mod & SDL_KMOD_ALT) != 0;

  if (has_primary && !has_shift && !has_alt) {  // Primary only
    switch (kbd_event.key) {
      case SDLK_O: event_queue_.push(CFR{.reason = CFR::Reason::ProjectOpen}); break;
      case SDLK_S: event_queue_.push(ProjectSaveRequest{}); break;

      default: break;
    }
  } else if (has_primary && has_shift && !has_alt) {  // Primary + Shift
    switch (kbd_event.key) {
      case SDLK_S: event_queue_.push(CFR{.reason = CFR::Reason::ProjectSaveAs}); break;

      default: break;
    }
  } else if (!has_primary && !has_shift && has_alt) {  // Alt only
    switch (kbd_event.key) {
      case SDLK_RETURN:
        event_queue_.push(RunRequest{.fragment_code = editor_.combined_code()});
        break;

      default: break;
    }
  }
}

}  // namespace mewo
