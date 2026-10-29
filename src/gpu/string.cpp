#include "gpu/string.hpp"

#include <optional>

namespace mewo {

std::optional<std::string> to_string(WGPUStringView view) {
  if (view.data == nullptr)
    return std::nullopt;
  if (view.length == 0)
    return "";

  return view.length == WGPU_STRLEN ? std::string(view.data) : std::string(view.data, view.length);
}

WGPUStringView to_string_view(std::string_view view) {
  WGPUStringView result = WGPU_STRING_VIEW_INIT;
  result.data = view.data();
  result.length = view.length();
  return result;
}

}  // namespace mewo
