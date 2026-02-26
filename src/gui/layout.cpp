#include "layout.hpp"

#include "aspect_ratio.hpp"
#include "utility.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <webgpu/webgpu.h>

#include <array>
#include <functional>
#include <string_view>
#include <utility>

namespace mewo::gui {

static constexpr std::string_view EDITOR_WINDOW_NAME = "Editor";
static constexpr std::string_view VIEWPORT_WINDOW_NAME = "Viewport";

void Layout::build(State& state, const Context& gui_ctx, const wgpu::Device& device, Editor& editor,
    Viewport& viewport)
{
  // Once the layout is created, the ID remains constant.
  if (const ImGuiID dockspace_id = ImGui::GetID("main-dockspace");
      ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
    set_up_initial_layout(gui_ctx, dockspace_id);
  } else {
    ImGui::DockSpaceOverViewport(
        dockspace_id, gui_ctx.viewport(), ImGuiDockNodeFlags_PassthruCentralNode);
  }

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Quit"))
        state.should_quit = true;

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  {
    ImGui::Begin(EDITOR_WINDOW_NAME.data());

    ImGui::PushFont(gui_ctx.fonts().geist_mono, 0.f);
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    ImGui::InputTextMultiline("##editor", &editor.visible_code(), window_size);
    ImGui::PopFont();

    ImGui::End();
  }

  {
    ImGui::Begin(VIEWPORT_WINDOW_NAME.data());

    const Viewport::Mode prev_mode = viewport.mode();
    const AspectRatio::Preset prev_preset = viewport.ratio_preset();
    const uint32_t prev_width = viewport.width();
    const uint32_t prev_height = viewport.height();

    const ImVec2 window_size = ImGui::GetContentRegionAvail();
    const auto curr_viewport_window_width = static_cast<uint32_t>(std::floor(window_size.x));

    // If the window containing the viewport has changed width, we resize the texture.
    // This only applies if the viewport mode is based on the aspect ratio.
    //
    // TODO: don't submit if user is actively dragging the window to be bigger/smaller
    if (prev_mode == Viewport::Mode::AspectRatio
        && curr_viewport_window_width != prev_viewport_window_width_) {
      viewport.set_pending_resize(curr_viewport_window_width);
    }

    {
      WGPUTextureView view_raw = viewport.view().Get();
      auto texture_id = static_cast<ImTextureID>(reinterpret_cast<intptr_t>(view_raw));

      auto inverse_ratio = std::invoke([&] -> float {
        switch (prev_mode) {
        case Viewport::Mode::AspectRatio:
          return AspectRatio::get_inverse_value(prev_preset);

        case Viewport::Mode::Resolution:
          // TODO: division by zero possible
          return static_cast<float>(prev_height) / static_cast<float>(prev_width);

        default:
          utility::enum_unreachable("Viewport::Mode", prev_mode);
        }
      });

      // Height of image is always derived from the width, because we horizontally fill the GUI
      ImGui::Image(texture_id, ImVec2(window_size.x, window_size.x * inverse_ratio));
    }

    if (ImGui::Button("Run")) {
      viewport.set_fragment_state(device, editor.combined_code());
      // TODO: only update render pipeline if shader compilation was successful
      viewport.update_render_pipeline(device);
    }

    {
      using Mode = Viewport::Mode;

      int prev_mode_value = std::to_underlying(prev_mode);

      ImGui::RadioButton("Aspect ratio", &prev_mode_value, std::to_underlying(Mode::AspectRatio));
      ImGui::SameLine();
      ImGui::RadioButton("Resolution", &prev_mode_value, std::to_underlying(Mode::Resolution));

      if (auto curr_mode = static_cast<Mode>(prev_mode_value); curr_mode != prev_mode) {
        viewport.set_mode(curr_mode);

        switch (curr_mode) {
        case Viewport::Mode::AspectRatio:
          viewport.set_pending_resize(curr_viewport_window_width);
          break;

        case Viewport::Mode::Resolution:
          viewport.set_pending_resize();
          break;

        default:
          utility::enum_unreachable("Viewport::Mode", curr_mode);
        }
      }
    }

    switch (prev_mode) {
    case Viewport::Mode::AspectRatio: {
      using Preset = AspectRatio::Preset;

      int prev_preset_value = std::to_underlying(prev_preset);

      ImGui::RadioButton("1:1", &prev_preset_value, std::to_underlying(Preset::e1_1));
      ImGui::SameLine();
      ImGui::RadioButton("2:1", &prev_preset_value, std::to_underlying(Preset::e2_1));
      ImGui::SameLine();
      ImGui::RadioButton("3:2", &prev_preset_value, std::to_underlying(Preset::e3_2));
      ImGui::SameLine();
      ImGui::RadioButton("16:9", &prev_preset_value, std::to_underlying(Preset::e16_9));

      if (auto curr_preset = static_cast<Preset>(prev_preset_value); curr_preset != prev_preset) {
        // Set ratio preset before submitting resize, because it has to use the new ratio
        viewport.set_ratio_preset(curr_preset);
        viewport.set_pending_resize(curr_viewport_window_width);
      }

      break;
    }

    case Viewport::Mode::Resolution: {
      static constexpr auto SLIDER_FLAGS = ImGuiSliderFlags_AlwaysClamp;
      static constexpr int VIEWPORT_SIZE_MIN = 2;
      static constexpr int VIEWPORT_SIZE_MAX = 2048;

      std::array prev_size = { static_cast<int>(prev_width), static_cast<int>(prev_height) };

      ImGui::DragInt2("Width/Height", prev_size.data(), 1.f, VIEWPORT_SIZE_MIN, VIEWPORT_SIZE_MAX,
          "%d px", SLIDER_FLAGS);

      uint32_t curr_width = static_cast<uint32_t>(prev_size[0]);
      uint32_t curr_height = static_cast<uint32_t>(prev_size[1]);

      // TODO: don't submit if user is currently selecting/dragging the slider, or has the
      //       box active and is still entering values
      if (curr_width != prev_width || curr_height != prev_height) {
        viewport.set_pending_resize(curr_width, curr_height);
        viewport.set_width(curr_width);
        viewport.set_height(curr_height);
      }

      break;
    }

    default:
      utility::enum_unreachable("Viewport::Mode", prev_mode);
    }

    prev_viewport_window_width_ = curr_viewport_window_width;

    ImGui::End();
  }
}

void Layout::set_up_initial_layout(const Context& gui_ctx, ImGuiID dockspace_id) const
{
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, gui_ctx.viewport()->Size);

  ImGuiID dock_left_id = {};
  ImGuiID dock_right_id = {};
  ImGui::DockBuilderSplitNode(
      dockspace_id, ImGuiDir_Left, SPLIT_LEFT_RATIO, &dock_left_id, &dock_right_id);

  ImGui::DockBuilderDockWindow(EDITOR_WINDOW_NAME.data(), dock_left_id);
  ImGui::DockBuilderDockWindow(VIEWPORT_WINDOW_NAME.data(), dock_right_id);

  ImGui::DockBuilderFinish(dockspace_id);
}

}
