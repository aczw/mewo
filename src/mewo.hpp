#pragma once

#include "editor.hpp"
#include "event/queue.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/gfx.hpp"
#include "gui/gui.hpp"
#include "pending.hpp"
#include "project.hpp"
#include "viewport.hpp"
#include "window.hpp"

#include <filesystem>
#include <optional>

namespace mewo {

class Mewo {
 public:
  Mewo();

  void run();

 private:
  const gfx::FrameContext prepare_new_frame();

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
