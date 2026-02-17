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
static constexpr std::string_view OUTPUT_WINDOW_NAME = "Output";

void Layout::build(
    const Context& gui_ctx, const wgpu::Device& device, Editor& editor, Out& out) const
{
  // Once the layout is created, the ID remains constant.
  if (const ImGuiID dockspace_id = ImGui::GetID("main-dockspace");
      ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
    set_up_initial_layout(gui_ctx, dockspace_id);
  } else {
    ImGui::DockSpaceOverViewport(
        dockspace_id, gui_ctx.viewport(), ImGuiDockNodeFlags_PassthruCentralNode);
  }

  {
    ImGui::Begin(EDITOR_WINDOW_NAME.data());

    ImGui::InputTextMultiline("##editor", &editor.code());

    ImGui::End();
  }

  {
    ImGui::Begin(OUTPUT_WINDOW_NAME.data());

    auto current_mode = out.display_mode();

    {
      WGPUTextureView view_raw = out.view().Get();
      auto texture_id = static_cast<ImTextureID>(reinterpret_cast<intptr_t>(view_raw));

      auto image_size = std::invoke([&out] -> ImVec2 {
        switch (out.display_mode()) {
        case Out::DisplayMode::AspectRatio: {
          float inverse_aspect_ratio = AspectRatio::get_inverse_value(out.aspect_ratio_preset());
          ImVec2 content_region = ImGui::GetContentRegionAvail();
          content_region.y = content_region.x * inverse_aspect_ratio;

          return content_region;
        }

        case Out::DisplayMode::Resolution:
          throw Exception("TODO: implement resolution display mode for output");

        default:
          utility::enum_unreachable("Out::DisplayMode", out.display_mode());
        }
      });

      ImGui::Image(texture_id, image_size);
    }

    if (ImGui::Button("Run")) {
      out.set_fragment_state(device, editor.code());
      out.update(device);
    }

    {
      using Mode = Out::DisplayMode;

      int mode_value = std::to_underlying(current_mode);

      ImGui::RadioButton("Aspect ratio", &mode_value, std::to_underlying(Mode::AspectRatio));
      ImGui::SameLine();
      ImGui::RadioButton("Resolution", &mode_value, std::to_underlying(Mode::Resolution));

      if (auto display_mode = static_cast<Mode>(mode_value); display_mode != current_mode)
        out.set_display_mode(display_mode);
    }

    {
      using Preset = AspectRatio::Preset;

      Preset current_preset = out.aspect_ratio_preset();
      int preset_value = std::to_underlying(current_preset);

      ImGui::RadioButton("1:1", &preset_value, std::to_underlying(Preset::e1_1));
      ImGui::SameLine();
      ImGui::RadioButton("2:1", &preset_value, std::to_underlying(Preset::e2_1));
      ImGui::SameLine();
      ImGui::RadioButton("3:2", &preset_value, std::to_underlying(Preset::e3_2));
      ImGui::SameLine();
      ImGui::RadioButton("16:9", &preset_value, std::to_underlying(Preset::e16_9));

      if (auto preset = static_cast<Preset>(preset_value); preset != current_preset)
        out.set_aspect_ratio_preset(preset);
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
  ImGui::DockBuilderDockWindow(OUTPUT_WINDOW_NAME.data(), dock_right_id);

  ImGui::DockBuilderFinish(dockspace_id);
}

}
