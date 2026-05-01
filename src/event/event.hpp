#pragma once

#include "aspect_ratio.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <variant>

namespace mewo {

struct QuitRequest {};

struct ChooseFolderRequest {
  enum class Reason : uint8_t {
    ProjectOpen,
    ProjectSaveAs,
  } reason = Reason::ProjectOpen;
};

struct ProjectOpenRequest {
  std::filesystem::path directory;
};

struct ProjectSaveAsRequest {
  std::filesystem::path directory;
};

struct ProjectSaveRequest {};

/// Can't resize the same frame because the viewport texture might already
/// be submitted for display in the GUI.
struct ViewportResizeRequest {
  uint32_t new_width = 0;
  uint32_t new_height = 0;

  /// Uses given width and derives the height from the aspect ratio.
  static ViewportResizeRequest from_width_and_ratio(
    uint32_t new_width,
    AspectRatio::Preset ratio_preset
  ) {
    float inverse_ratio = AspectRatio::get_inverse_value(ratio_preset);
    float height = std::floor(static_cast<float>(new_width) * inverse_ratio);
    return {.new_width = new_width, .new_height = static_cast<uint32_t>(height)};
  }
};

struct ViewportPlaybackToggled {};

struct ViewportPlaybackTimeResetRequest {};

struct RunRequest {
  std::string fragment_code;  ///< Combined fragment shader.
};

struct WindowResized {
  uint32_t new_width = 0;
  uint32_t new_height = 0;
};

struct WGPUDeviceLost {
  std::string_view reason;
  std::string message;
};

struct WGPUUncapturedError {
  std::string_view type_name;
  std::string message;
};

struct EditorTextChanged {};

struct AutoCompilerElapsed {};

struct HotReloadingToggled {};

using Event = std::variant<
  const QuitRequest,
  const ChooseFolderRequest,
  const ProjectOpenRequest,
  const ProjectSaveAsRequest,
  const ProjectSaveRequest,
  const ViewportResizeRequest,
  const ViewportPlaybackToggled,
  const ViewportPlaybackTimeResetRequest,
  const RunRequest,
  const WindowResized,
  const WGPUDeviceLost,
  const WGPUUncapturedError,
  const EditorTextChanged,
  const AutoCompilerElapsed,
  const HotReloadingToggled
>;

}  // namespace mewo
