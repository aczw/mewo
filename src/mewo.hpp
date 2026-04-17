#pragma once

#include "event/event_queue.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/gfx.hpp"
#include "gui/editor/auto_compiler.hpp"
#include "gui/editor/editor.hpp"
#include "gui/gui.hpp"
#include "project.hpp"
#include "viewport.hpp"
#include "window.hpp"

#include <SDL3/SDL_dialog.h>

#include <cstdint>
#include <filesystem>
#include <optional>

namespace mewo {

class Mewo {
 public:
  Mewo();

  void run();

 private:
  void process_queued_events();
  /// `delta_time` is expressed in milliseconds.
  void update(const gfx::FrameContext& frame_ctx, uint64_t delta_time);

  std::filesystem::path executable_dir_;
  std::filesystem::path assets_dir_;
  bool should_quit_ = false;

  EventQueue event_queue_;

  Window window_;
  gfx::Gfx gfx_;
  Gui gui_;
  Editor editor_;
  Viewport viewport_;
  AutoCompiler auto_compiler_;

  uint64_t current_time_ = 0;  ///< Milliseconds since `SDL_Init()` has been called.
  std::optional<Project> project_;
};

}  // namespace mewo
