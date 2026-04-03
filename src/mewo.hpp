#pragma once

#include "editor.hpp"
#include "event/event_queue.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/gfx.hpp"
#include "gui/gui.hpp"
#include "project.hpp"
#include "viewport.hpp"
#include "window.hpp"

#include <SDL3/SDL_dialog.h>

#include <filesystem>
#include <optional>

namespace mewo {

class Mewo {
 public:
  Mewo();

  void run();

 private:
  void process_queued_events();
  void update(const gfx::FrameContext& frame_ctx);

  std::filesystem::path executable_dir_;
  std::filesystem::path assets_dir_;
  bool should_quit_ = false;

  EventQueue event_queue_;

  Window window_;
  gfx::Gfx gfx_;
  Gui gui_;
  Editor editor_;
  Viewport viewport_;
  std::optional<Project> project_;
};

}  // namespace mewo
