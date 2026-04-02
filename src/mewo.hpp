#pragma once

#include "editor.hpp"
#include "event/event.hpp"
#include "event/queue.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/gfx.hpp"
#include "gui/gui.hpp"
#include "pending.hpp"
#include "project.hpp"
#include "viewport.hpp"
#include "window.hpp"

#include <SDL3/SDL_dialog.h>

#include <filesystem>
#include <optional>
#include <print>

namespace mewo {

class Mewo {
 public:
  Mewo();

  void run();

 private:
  void process_queued_events();
  void apply_pending_actions();
  void update(const gfx::FrameContext& frame_ctx);

  template <event::ChooseFolderRequest::Reason Reason>
  void show_open_folder_dialog() {
    // TODO: probably can't use this for macOS because it doesn't let you create a
    // folder from within the dialog by default
    SDL_ShowOpenFolderDialog(
      [](void* userdata, const char* const* filelist, int) -> void {
        if (filelist == nullptr) {
          std::println("Error while showing folder dialog: {}", SDL_GetError());
          return;
        }

        if (*filelist == nullptr)
          return;

        const char* folder_path = nullptr;
        int count = 0;

        while (*filelist) {
          folder_path = *filelist;
          filelist += 1;
          count += 1;
        }

        if (count > 1)
          std::println("warning: more than one folder selected, using last one");

        auto& event_queue = static_cast<Mewo*>(userdata)->event_queue_;
        using CFR = event::ChooseFolderRequest;

        if constexpr (Reason == CFR::Reason::ProjectOpen) {
          event_queue.push(event::ProjectOpenRequest{.directory = folder_path});
        } else if constexpr (Reason == CFR::Reason::ProjectSaveAs) {
          event_queue.push(event::ProjectSaveAsRequest{.directory = folder_path});
        } else {
          static_assert(false, "Unhandled event::ChooseFolderRequest::Reason case");
        }
      },
      this,
      window_.get(),
      nullptr,
      false
    );
  }

  std::filesystem::path executable_dir_;
  std::filesystem::path assets_dir_;
  bool should_quit_ = false;

  event::Queue event_queue_;

  Pending pending_;

  Window window_;
  gfx::Gfx gfx_;
  Gui gui_;

  Editor editor_;
  Viewport viewport_;

  std::optional<Project> project_;
};

}  // namespace mewo
