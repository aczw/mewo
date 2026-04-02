#pragma once

#include "aspect_ratio.hpp"

#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace mewo {

/// Collects pending operations to be applied and processed in the next frame.
class Pending {
 public:
  std::optional<std::pair<uint32_t, uint32_t>>& viewport_resize() { return viewport_resize_; }

  std::optional<std::string>& run() { return run_; }

  /// Uses given width and derives the height from the aspect ratio.
  void request_viewport_resize(uint32_t new_width, AspectRatio::Preset ratio_preset) {
    float inverse_ratio = AspectRatio::get_inverse_value(ratio_preset);
    float height = std::floor(static_cast<float>(new_width) * inverse_ratio);
    viewport_resize_ = {new_width, static_cast<uint32_t>(height)};
  }

  /// Uses given width and height.
  void request_viewport_resize(uint32_t new_width, uint32_t new_height) {
    viewport_resize_ = {new_width, new_height};
  }

  void request_run(std::string_view new_combined_code) { run_ = std::string(new_combined_code); }

 private:
  /// Can't resize the same frame because the viewport texture might already
  /// be submitted for display in the GUI.
  std::optional<std::pair<uint32_t, uint32_t>> viewport_resize_;
  /// Stores combined fragment shader.
  std::optional<std::string> run_;
};

}  // namespace mewo
