#pragma once

#include "editor.hpp"
#include "gfx/frame_context.hpp"
#include "gfx/gfx.hpp"
#include "pending.hpp"
#include "project.hpp"
#include "ui/layout.hpp"
#include "viewport.hpp"
#include "window.hpp"

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

  Window window_;
  gfx::Gfx gfx_;
  gui::Layout layout_;

  Editor editor_;
  Viewport viewport_;

  std::optional<Project> project_;
};

}  // namespace mewo
