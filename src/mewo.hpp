#pragma once

#include "editor.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/renderer.hpp"
#include "gui/context.hpp"
#include "gui/layout.hpp"
#include "pending.hpp"
#include "project.hpp"
#include "sdl/context.hpp"
#include "sdl/window.hpp"
#include "viewport.hpp"

#include <filesystem>
#include <optional>

namespace mewo {

class Mewo {
 public:
  Mewo();

  void run();
  const gfx::FrameContext prepare_new_frame();

 private:
  std::filesystem::path executable_dir_;
  std::filesystem::path assets_dir_;

  Pending pending_;

  sdl::Context sdl_ctx_;
  sdl::Window window_;

  gfx::Renderer renderer_;

  gui::Context gui_ctx_;
  gui::Layout layout_;

  Editor editor_;
  Viewport viewport_;

  std::optional<Project> project_;
};

}  // namespace mewo
