#include "gui.hpp"

#include "aspect_ratio.hpp"
#include "event/event.hpp"
#include "gui/editor/editor.hpp"
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
#include <functional>
#include <string_view>
#include <utility>

namespace mewo {

static constexpr std::string_view EDITOR_WINDOW_NAME = "Editor";
static constexpr std::string_view DIAGNOSTICS_WINDOW_NAME = "Diagnostics";
static constexpr std::string_view VIEWPORT_WINDOW_NAME = "Viewport";

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
  viewport_ = ImGui::GetMainViewport();
}

void Gui::build_layout(
  EventQueue& event_queue,
  const gfx::FrameContext& frame_ctx,
  Editor& editor,
  Viewport& viewport
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
      viewport_,
      ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking
    );
  }

  build_main_menu_bar(event_queue, editor);
  build_editor(editor);
  build_diagnostics(editor);
  build_viewport(event_queue, frame_ctx, viewport, editor);
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

void Gui::build_main_menu_bar(EventQueue& event_queue, Editor& editor) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      using CFR = ChooseFolderRequest;

      if (ImGui::MenuItem("Open..."))
        event_queue.push(CFR{.reason = CFR::Reason::ProjectOpen});

      ImGui::Separator();

      if (ImGui::MenuItem("Save"))
        event_queue.push(ProjectSaveRequest{});

      if (ImGui::MenuItem("Save As..."))
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

    ImGui::EndMainMenuBar();
  }
}

void Gui::build_editor(Editor& editor) const {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 6.f));
  // TODO: while this lets the padding blend in with the background, when scrolling you
  // can clearly see the padding because the text is truncated early
  ImGui::PushStyleColor(ImGuiCol_WindowBg, editor.palette().get(TextEditor::Color::background));

  ImGui::Begin(
    EDITOR_WINDOW_NAME.data(),
    nullptr,
    editor.dirty() ? ImGuiWindowFlags_UnsavedDocument : ImGuiWindowFlags_None
  );

  {
    ImGui::PushFont(fonts_.geist_mono, 0.f);
    {
      editor.build_layout(ImGui::GetContentRegionAvail());
    }
    ImGui::PopFont();
  }

  ImGui::End();

  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
}

void Gui::build_diagnostics(Editor& editor) const {
  ImGui::Begin(DIAGNOSTICS_WINDOW_NAME.data());

  ImGui::PushFont(fonts_.geist_mono, 0.f);
  {
    if (const auto& diagnostics = editor.diagnostics(); diagnostics.size() > 0) {
      auto prefix_line_count = static_cast<uint64_t>(editor.prefix_line_count());

      for (const auto& diag : diagnostics) {
        // Subtract away the number of lines the fragment prefix takes up as it's not visible
        uint64_t line_number = diag.line_number - prefix_line_count;

        ImGui::Text(
          "(Ln %llu, Col %llu) %s: %s",
          line_number,
          diag.line_column,
          diag.type_name.data(),
          diag.message.c_str()
        );
        ImGui::Text("%s", diag.highlight.c_str());

        std::string indicators;
        for (size_t i = 0; i < diag.highlight.size(); ++i)
          indicators += '^';
        ImGui::Text("%s", indicators.c_str());

        // TODO: don't include spacing if it's the last diagnostic
        ImGui::Spacing();
        ImGui::Spacing();
      }
    } else {
      ImGui::Text("Compilation succeeded with no issues.");
    }
  }
  ImGui::PopFont();

  ImGui::End();
}

void Gui::build_viewport(
  EventQueue& event_queue,
  const gfx::FrameContext& frame_ctx,
  Viewport& viewport,
  Editor& editor
) {
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

    // Height of image is always derived from the width, because we horizontally fill the GUI
    ImGui::Image(texture_id, ImVec2(window_size.x, window_size.x * inverse_ratio));
  }

  if (ImGui::Button("Run"))
    event_queue.push(RunRequest{.fragment_code = editor.combined_code()});

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

      ImGui::DragInt2(
        "Width/Height",
        prev_size.data(),
        1.f,
        VIEWPORT_SIZE_MIN,
        VIEWPORT_SIZE_MAX,
        "%d px",
        SLIDER_FLAGS
      );

      uint32_t curr_width = static_cast<uint32_t>(prev_size[0]);
      uint32_t curr_height = static_cast<uint32_t>(prev_size[1]);

      // TODO: don't submit if user is currently selecting/dragging the slider, or has the
      // box active and is still entering values
      if (curr_width != prev_width || curr_height != prev_height) {
        event_queue.push(ViewportResizeRequest{.new_width = curr_width, .new_height = curr_height});
        viewport.set_resolution(curr_width, curr_height);
      }

      break;
    }

    default: util::enum_unreachable("Viewport::Mode", prev_mode);
  }

  prev_viewport_window_width_ = curr_viewport_window_width;

  ImGui::End();
}

void Gui::set_up_initial_layout(ImGuiID dockspace_id) const {
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, viewport_->Size);

  ImGuiID left_id = {};
  ImGuiID right_id = {};
  ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, SPLIT_LEFT_RATIO, &left_id, &right_id);

  ImGuiID left_up_id = {};
  ImGuiID left_down_id = {};
  ImGui::DockBuilderSplitNode(left_id, ImGuiDir_Up, 0.75f, &left_up_id, &left_down_id);

  ImGui::DockBuilderDockWindow(EDITOR_WINDOW_NAME.data(), left_up_id);
  ImGui::DockBuilderDockWindow(DIAGNOSTICS_WINDOW_NAME.data(), left_down_id);
  ImGui::DockBuilderDockWindow(VIEWPORT_WINDOW_NAME.data(), right_id);

  ImGui::DockBuilderGetNode(right_id)->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);
  ImGui::DockBuilderGetNode(left_up_id)->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);
  ImGui::DockBuilderGetNode(left_down_id)->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);

  ImGui::DockBuilderFinish(dockspace_id);
}

}  // namespace mewo
