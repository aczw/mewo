#include "layout.hpp"

#include "aspect_ratio.hpp"
#include "exception.hpp"
#include "utility.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <webgpu/webgpu.h>

#include <functional>
#include <string>
#include <string_view>
#include <utility>

namespace mewo::gui {

static constexpr std::string_view EDITOR_WINDOW_NAME = "Editor";
static constexpr std::string_view VIEWPORT_WINDOW_NAME = "Viewport";

void Layout::build(State& state, const Context& gui_ctx, const wgpu::Device& device, Editor& editor,
    Viewport& viewport) const
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

    ImVec2 window_size = ImGui::GetContentRegionAvail();
    ImGui::InputTextMultiline("##editor", &editor.code(), window_size);

    ImGui::End();
  }

  {
    ImGui::Begin(VIEWPORT_WINDOW_NAME.data());

    Viewport::Mode curr_mode = viewport.mode();
    AspectRatio::Preset curr_preset = viewport.ratio_preset();

    ImVec2 window_size = ImGui::GetContentRegionAvail();
    state.prev_viewport_window_width = state.curr_viewport_window_width;
    state.curr_viewport_window_width = static_cast<uint32_t>(window_size.x);

    {
      WGPUTextureView view_raw = viewport.view().Get();
      auto texture_id = static_cast<ImTextureID>(reinterpret_cast<intptr_t>(view_raw));

      auto image_size = std::invoke([&curr_mode, &curr_preset, &window_size] -> ImVec2 {
        switch (curr_mode) {
        case Viewport::Mode::AspectRatio: {
          window_size.y = window_size.x * AspectRatio::get_inverse_value(curr_preset);
          return window_size;
        }

        case Viewport::Mode::Resolution:
          throw Exception("TODO: implement resolution display mode for output");

        default:
          utility::enum_unreachable("Viewport::Mode", curr_mode);
        }
      });

      ImGui::Image(texture_id, image_size);
    }

    if (ImGui::Button("Run")) {
      viewport.set_fragment_state(device, editor.code());
      viewport.update(device);
    }

    {
      using Mode = Viewport::Mode;

      int mode_value = std::to_underlying(curr_mode);

      ImGui::RadioButton("Aspect ratio", &mode_value, std::to_underlying(Mode::AspectRatio));
      ImGui::SameLine();
      ImGui::RadioButton("Resolution", &mode_value, std::to_underlying(Mode::Resolution));

      if (auto mode = static_cast<Mode>(mode_value); mode != curr_mode)
        viewport.set_mode(mode);
    }

    {
      using Preset = AspectRatio::Preset;

      int preset_value = std::to_underlying(curr_preset);

      ImGui::RadioButton("1:1", &preset_value, std::to_underlying(Preset::e1_1));
      ImGui::SameLine();
      ImGui::RadioButton("2:1", &preset_value, std::to_underlying(Preset::e2_1));
      ImGui::SameLine();
      ImGui::RadioButton("3:2", &preset_value, std::to_underlying(Preset::e3_2));
      ImGui::SameLine();
      ImGui::RadioButton("16:9", &preset_value, std::to_underlying(Preset::e16_9));

      if (auto preset = static_cast<Preset>(preset_value); preset != curr_preset)
        viewport.set_ratio_preset(preset);
    }

    ImGui::End();
  }
}

void Layout::set_up_initial_layout(const Context& gui_ctx, ImGuiID dockspace_id) const
{
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, gui_ctx.viewport()->Size);

  ImGuiID dock_left_id = {};
  ImGuiID dock_right_id = {};
  ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.5f, &dock_left_id, &dock_right_id);

  ImGui::DockBuilderDockWindow(EDITOR_WINDOW_NAME.data(), dock_left_id);
  ImGui::DockBuilderDockWindow(VIEWPORT_WINDOW_NAME.data(), dock_right_id);

  ImGui::DockBuilderFinish(dockspace_id);
}

}
