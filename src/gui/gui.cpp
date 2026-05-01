#include "gui.hpp"

#include "aspect_ratio.hpp"
#include "event/event.hpp"
#include "gui/editor/editor.hpp"
#include "os.hpp"
#include "util/enum_unreachable.hpp"

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_error.h>
#include <TextEditor.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <webgpu/webgpu.h>

#include <array>
#include <cfloat>
#include <format>
#include <functional>
#include <string_view>
#include <utility>

namespace mewo {

namespace {

constexpr std::string_view LEFT_HALF_WINDOW_NAME = "Editor";
constexpr std::string_view VIEWPORT_WINDOW_NAME = "Viewport";

enum class ModKey {
  Primary,
  Alt,
};

/// Generate shortcut label at compile time for the given keys.
template <ModKey Key, size_t N>
consteval auto make_shortcut(const char (&keys)[N]) {
  constexpr auto PREFIX = std::invoke([] -> std::string_view {
    if constexpr (Key == ModKey::Primary) {
      return os::PRIMARY_MOD_LABEL;
    } else if constexpr (Key == ModKey::Alt) {
      return os::ALT_MOD_LABEL;
    } else {
      static_assert(Key == ModKey::Primary || Key == ModKey::Alt, "Unhandled mod key");
    }

    return "<None>";
  });

  std::array<char, PREFIX.size() + 1 + N> label = {};  // Prefix length + "+" + keys

  size_t index = 0;
  for (char character : PREFIX)
    label[index++] = character;

  label[index++] = '+';

  for (size_t j = 0; j < N; ++j)
    label[index++] = keys[j];

  return label;
}

template <size_t N>
consteval auto primary_shortcut(const char (&keys)[N]) {
  return make_shortcut<ModKey::Primary>(keys);
}

template <size_t N>
consteval auto alt_shortcut(const char (&keys)[N]) {
  return make_shortcut<ModKey::Alt>(keys);
}

}  // namespace

Gui::Gui(const std::filesystem::path& assets_dir, const Window& window, const gfx::Gfx& gfx) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.IniFilename = nullptr;
  io.ConfigDpiScaleFonts = true;
  io.ConfigDpiScaleViewports = true;

  auto inter_path = assets_dir / "fonts/inter_4.1/Inter-Regular.ttf";
  auto geist_mono_path = assets_dir / "fonts/geist_mono_1.7/GeistMono-Regular.ttf";

  fonts_ = {
    .inter = io.Fonts->AddFontFromFileTTF(inter_path.string().c_str()),
    .geist_mono = io.Fonts->AddFontFromFileTTF(geist_mono_path.string().c_str()),
  };

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.FontSizeBase = 15.f;

  ImGui_ImplWGPU_InitInfo wgpu_init_info;
  wgpu_init_info.Device = gfx.device().Get();
  wgpu_init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(gfx.surface_config().format);
  ImGui_ImplWGPU_Init(&wgpu_init_info);

  ImGui_ImplSDL3_InitForOther(window.get());

  // Can be set once upfront because there's only one viewport
  imgui_viewport_ = ImGui::GetMainViewport();
}

void Gui::build_layout(
  EventQueue& event_queue,
  const gfx::FrameContext& frame_ctx,
  Editor& editor,
  Viewport& viewport,
  const std::optional<AutoCompiler>& auto_compiler
) {
  // Once the layout is created, the ID remains constant.
  if (
    const ImGuiID dockspace_id = ImGui::GetID("main-dockspace");
    ImGui::DockBuilderGetNode(dockspace_id) == nullptr
  ) {
    set_up_initial_layout(dockspace_id);
  } else {
    ImGui::DockSpaceOverViewport(
      dockspace_id,
      imgui_viewport_,
      ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking
    );
  }

  build_main_menu_bar(event_queue, editor, auto_compiler);
  build_left_half(event_queue, editor);
  build_viewport(event_queue, frame_ctx, viewport);
}

void Gui::update(const gfx::FrameContext& frame_ctx) const {
  ImGui::Render();

  wgpu::RenderPassColorAttachment color_attachment = {
    .view = frame_ctx.surface_view,
    .loadOp = wgpu::LoadOp::Load,
    .storeOp = wgpu::StoreOp::Store,
  };

  wgpu::RenderPassDescriptor render_pass_desc = {
    .label = "imgui-render-pass",
    .colorAttachmentCount = 1,
    .colorAttachments = &color_attachment,
  };

  wgpu::RenderPassEncoder render_pass = frame_ctx.encoder.BeginRenderPass(&render_pass_desc);
  ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), render_pass.Get());
  render_pass.End();
}

void Gui::build_main_menu_bar(
  EventQueue& event_queue,
  Editor& editor,
  const std::optional<AutoCompiler>& auto_compiler
) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      using CFR = ChooseFolderRequest;

      if (ImGui::MenuItem("Open...", primary_shortcut("O").data()))
        event_queue.push(CFR{.reason = CFR::Reason::ProjectOpen});

      ImGui::Separator();

      if (ImGui::MenuItem("Save", primary_shortcut("S").data()))
        event_queue.push(ProjectSaveRequest{});

      if (ImGui::MenuItem("Save As...", primary_shortcut("Shift+S").data()))
        event_queue.push(CFR{.reason = CFR::Reason::ProjectSaveAs});

      ImGui::Separator();

      if (ImGui::MenuItem("Quit"))
        event_queue.push(QuitRequest{});

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
      ImGui::SeparatorText("Theme");

      {
        Theme curr_theme = theme_;

        if (ImGui::MenuItem("Default Dark", nullptr, curr_theme == Theme::DefaultDark))
          curr_theme = Theme::DefaultDark;
        if (ImGui::MenuItem("Default Light", nullptr, curr_theme == Theme::DefaultLight))
          curr_theme = Theme::DefaultLight;
        if (ImGui::MenuItem("Rosé Pine", nullptr, curr_theme == Theme::RosePine))
          curr_theme = Theme::RosePine;

        if (curr_theme != theme_) {
          theme_ = curr_theme;
          editor.update_theme(theme_);
        }
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Run")) {
      if (ImGui::MenuItem("Compile", alt_shortcut("Enter").data()))
        event_queue.push(RunRequest{.fragment_code = editor.combined_code()});

      ImGui::Separator();

      if (ImGui::MenuItem("Hot reloading", nullptr, auto_compiler.has_value()))
        event_queue.push(HotReloadingToggled{});

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

void Gui::build_left_half(EventQueue& event_queue, Editor& editor) {
  const auto& style = ImGui::GetStyle();

  ImVec2 window_padding = style.WindowPadding;
  ImVec2 status_bar_window_padding = ImVec2(window_padding.x, 0.5f * window_padding.y);
  ImVec2 diagnostics_size = ImVec2(0.f, 200.f);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
  ImGui::Begin(LEFT_HALF_WINDOW_NAME.data());

  ImGui::PushFont(fonts_.geist_mono, 0.f);
  {
    float status_bar_height = ImGui::GetFrameHeight() + (2.f * status_bar_window_padding.y);
    auto editor_size = ImVec2(
      0.f,
      ImGui::GetContentRegionAvail().y - status_bar_height -
        (is_diagnostics_visible_ ? diagnostics_size.y + style.ItemSpacing.y : 0.f)
    );

    editor.build_layout(editor_size);
  }
  ImGui::PopFont();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, status_bar_window_padding);
  {
    if (is_diagnostics_visible_)
      build_diagnostics(editor, diagnostics_size);

    ImGui::BeginChild("##Status Bar", ImVec2(), ImGuiChildFlags_AlwaysUseWindowPadding);
    {
      ImGui::AlignTextToFramePadding();

      if (ImGui ::Button("► Run"))
        event_queue.push(RunRequest{.fragment_code = editor.combined_code()});

      ImGui::SameLine();

      if (
        auto num_diagnostics = editor.diagnostics().size(); ImGui::Button(
          std::format("{} diagnostic{}", num_diagnostics, num_diagnostics == 1 ? "" : "s").c_str()
        )
      ) {
        is_diagnostics_visible_ = !is_diagnostics_visible_;
      }

      ImGui::SameLine();

      if (is_last_compilation_successful_) {
        static constexpr double SECONDS_TO_MILLISECONDS = 1000.0;
        ImGui::Text(
          "Compiled in %.2f ms.", last_compilation_duration_.count() * SECONDS_TO_MILLISECONDS
        );
      } else {
        ImGui::TextUnformatted("Compilation failed.");
      }

      int line = 0, column = 0;
      editor.impl().GetCurrentCursor(line, column);
      std::string text = std::format("Ln {}, Col {}", line + 1, column + 1);

      ImGui::SameLine();
      ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
        ImGui::CalcTextSize(text.c_str()).x
      );
      ImGui::TextUnformatted(text.c_str());
    }
    ImGui::EndChild();
  }
  ImGui::PopStyleVar();

  ImGui::End();
  ImGui::PopStyleVar();
}

void Gui::build_diagnostics(Editor& editor, const ImVec2& size) const {
  ImGui::BeginChild("##Diagnostics", size, ImGuiChildFlags_AlwaysUseWindowPadding);

  if (const auto& diagnostics = editor.diagnostics(); diagnostics.size() > 0) {
    auto prefix_line_count = static_cast<uint64_t>(editor.prefix_line_count());

    for (const auto& [message, type_name, line_number, line_column, code_highlight] : diagnostics) {
      ImGui::SeparatorText(
        std::format("Ln {}, Col {}", line_number - prefix_line_count, line_column).c_str()
      );

      ImGui::PushFont(fonts_.geist_mono, 0.f);
      {
        ImGui::TextUnformatted(code_highlight.c_str());
      }
      ImGui::PopFont();

      ImGui::TextWrapped("%s: %s", type_name.data(), message.c_str());
    }
  } else {
    ImGui::Text("Compilation succeeded with no issues.");
  }

  ImGui::EndChild();
}

void Gui::build_viewport(
  EventQueue& event_queue,
  const gfx::FrameContext& frame_ctx,
  Viewport& viewport
) {
  const auto& style = ImGui::GetStyle();
  const ImVec2 window_padding = style.WindowPadding;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
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
  // Also, skip resizing if we're on the first few frames because Dear ImGui seems to not have
  // finished calculating layout widths.
  //
  // TODO: don't submit if user is actively dragging the window to be bigger/smaller
  if (
    frame_ctx.number > 3 && prev_mode == Viewport::Mode::AspectRatio &&
    curr_viewport_window_width != prev_viewport_window_width_
  ) {
    event_queue.push(
      ViewportResizeRequest::from_width_and_ratio(
        curr_viewport_window_width, viewport.ratio_preset()
      )
    );
  }

  {
    WGPUTextureView view_raw = viewport.view().Get();
    auto texture_id = static_cast<ImTextureID>(reinterpret_cast<intptr_t>(view_raw));

    auto inverse_ratio = std::invoke([&] -> float {
      switch (prev_mode) {
        case Viewport::Mode::AspectRatio: return AspectRatio::get_inverse_value(prev_preset);

        case Viewport::Mode::Resolution:
          // TODO: division by zero possible
          return static_cast<float>(prev_height) / static_cast<float>(prev_width);

        default: util::enum_unreachable("Viewport::Mode", prev_mode);
      }
    });

    auto image_size = ImVec2(window_size.x, window_size.x * inverse_ratio);

    // Height of image is always derived from the width, because we horizontally fill the GUI
    if (ImGui::Image(texture_id, image_size); ImGui::IsItemHovered()) {
      ImVec2 upper_left = ImGui::GetItemRectMin();
      ImVec2 mouse_pos = ImGui::GetIO().MousePos;

      auto relative_pos =
        ImVec2(mouse_pos.x - upper_left.x, std::floor(image_size.y) - (mouse_pos.y - upper_left.y));

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        viewport.set_mouse_pos_during_last_down(relative_pos);
        viewport.set_mouse_pos_during_last_click(relative_pos);
      } else {
        ImVec2 mouse_pos_during_last_click = viewport.mouse_pos_during_last_click();

        if (mouse_pos_during_last_click.x > 0.f)
          mouse_pos_during_last_click.x *= -1.f;
        if (mouse_pos_during_last_click.y > 0.f)
          mouse_pos_during_last_click.y *= -1.f;

        viewport.set_mouse_pos_during_last_click(mouse_pos_during_last_click);
      }

      if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        viewport.set_mouse_pos_during_last_down(relative_pos);
      }
    }
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, window_padding);
  ImGui::BeginChild("##Viewport Controls", ImVec2(), ImGuiChildFlags_AlwaysUseWindowPadding);
  {
    if (
      ImGui::Button(
        viewport.is_playing() ? "Pause" : "Play",
        ImVec2(ImGui::CalcTextSize("Pause").x + 2.f * style.FramePadding.x, 0.f)
      )
    ) {
      event_queue.push(ViewportPlaybackToggled{});
    }
    ImGui::SameLine();

    if (ImGui::Button("↺"))
      event_queue.push(ViewportPlaybackTimeResetRequest{});
    ImGui::SameLine();

    ImGui::Text("%.1f", viewport.current_time());
    ImGui::SameLine();

    {
      std::string text = std::format("{:.0f} FPS", ImGui::GetIO().Framerate);
      float text_width = ImGui::CalcTextSize(text.c_str()).x;
      float controls_button_width = ImGui::CalcTextSize("Controls").x + 2.f * style.FramePadding.x;
      ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - text_width -
        style.ItemSpacing.x - controls_button_width
      );

      ImGui::TextUnformatted(text.c_str());
      ImGui::SameLine();
    }

    static constexpr std::string_view VIEWPORT_CONTROLS_LABEL = "##Viewport Controls";
    if (ImGui::Button("Controls"))
      ImGui::OpenPopup(VIEWPORT_CONTROLS_LABEL.data(), ImGuiPopupFlags_NoReopen);

    if (ImGui::BeginPopup(VIEWPORT_CONTROLS_LABEL.data())) {
      {
        using Mode = Viewport::Mode;

        int prev_mode_value = std::to_underlying(prev_mode);

        ImGui::RadioButton("Aspect ratio", &prev_mode_value, std::to_underlying(Mode::AspectRatio));
        ImGui::SameLine();
        ImGui::RadioButton("Resolution", &prev_mode_value, std::to_underlying(Mode::Resolution));

        if (auto curr_mode = static_cast<Mode>(prev_mode_value); curr_mode != prev_mode) {
          viewport.set_mode(curr_mode);

          switch (curr_mode) {
            case Viewport::Mode::AspectRatio: {
              event_queue.push(
                ViewportResizeRequest::from_width_and_ratio(
                  curr_viewport_window_width, viewport.ratio_preset()
                )
              );
              break;
            }

            case Viewport::Mode::Resolution: {
              event_queue.push(
                ViewportResizeRequest{.new_width = prev_width, .new_height = prev_height}
              );
              break;
            }

            default: util::enum_unreachable("Viewport::Mode", curr_mode);
          }
        }
      }

      ImGui::Separator();

      switch (prev_mode) {
        case Viewport::Mode::AspectRatio: {
          using Preset = AspectRatio::Preset;

          int prev_preset_value = std::to_underlying(prev_preset);

          ImGui::RadioButton("1:1", &prev_preset_value, std::to_underlying(Preset::e1_1));
          ImGui::RadioButton("2:1", &prev_preset_value, std::to_underlying(Preset::e2_1));
          ImGui::RadioButton("3:2", &prev_preset_value, std::to_underlying(Preset::e3_2));
          ImGui::RadioButton("16:9", &prev_preset_value, std::to_underlying(Preset::e16_9));

          if (
            auto curr_preset = static_cast<Preset>(prev_preset_value); curr_preset != prev_preset
          ) {
            event_queue.push(
              ViewportResizeRequest::from_width_and_ratio(curr_viewport_window_width, curr_preset)
            );
            viewport.set_ratio_preset(curr_preset);
          }

          break;
        }

        case Viewport::Mode::Resolution: {
          static constexpr auto SLIDER_FLAGS = ImGuiSliderFlags_AlwaysClamp;
          static constexpr int VIEWPORT_SIZE_MIN = 2;
          static constexpr int VIEWPORT_SIZE_MAX = 2048;

          std::array prev_size = {static_cast<int>(prev_width), static_cast<int>(prev_height)};

          ImGui::PushItemWidth(-FLT_MIN);
          ImGui::DragInt2(
            "##Width/Height",
            prev_size.data(),
            1.f,
            VIEWPORT_SIZE_MIN,
            VIEWPORT_SIZE_MAX,
            "%d px",
            SLIDER_FLAGS
          );
          ImGui::PopItemWidth();

          uint32_t curr_width = static_cast<uint32_t>(prev_size[0]);
          uint32_t curr_height = static_cast<uint32_t>(prev_size[1]);

          // TODO: don't submit if user is currently selecting/dragging the slider, or has the
          // box active and is still entering values
          if (curr_width != prev_width || curr_height != prev_height) {
            event_queue.push(
              ViewportResizeRequest{.new_width = curr_width, .new_height = curr_height}
            );
            viewport.set_resolution(curr_width, curr_height);
          }

          break;
        }

        default: util::enum_unreachable("Viewport::Mode", prev_mode);
      }

      ImGui::EndPopup();
    }
  }
  ImGui::EndChild();
  ImGui::PopStyleVar();

  prev_viewport_window_width_ = curr_viewport_window_width;

  ImGui::End();
  ImGui::PopStyleVar();
}

void Gui::set_up_initial_layout(ImGuiID dockspace_id) const {
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, imgui_viewport_->Size);

  ImGuiID left_id = {};
  ImGuiID right_id = {};
  ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, SPLIT_LEFT_RATIO, &left_id, &right_id);

  ImGui::DockBuilderDockWindow(LEFT_HALF_WINDOW_NAME.data(), left_id);
  ImGui::DockBuilderDockWindow(VIEWPORT_WINDOW_NAME.data(), right_id);

  ImGui::DockBuilderGetNode(right_id)->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);
  ImGui::DockBuilderGetNode(left_id)->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);

  ImGui::DockBuilderFinish(dockspace_id);
}

}  // namespace mewo
