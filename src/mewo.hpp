#pragma once

#include "assets.hpp"
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

#include <optional>

namespace mewo {

class Mewo {
 public:
  Mewo()
      : renderer_(window_),
        gui_ctx_(assets_, window_, renderer_),
        editor_(assets_),
        viewport_(pending_, assets_, renderer_, editor_.combined_code()) {}

  void run();
  const gfx::FrameContext prepare_new_frame();

 private:
  Pending pending_;
  Assets assets_;

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
