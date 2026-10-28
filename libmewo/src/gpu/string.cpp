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

}  // namespace mewo
