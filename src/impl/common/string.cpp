#include "impl/common/string.hpp"

#include <optional>
#include <string>

namespace mewo::impl {

std::optional<std::string> to_string(mewo_string_view_t view) {
  if (view.data == nullptr && view.length != 0)
    return std::nullopt;

  if (view.length == 0)
    return "";

  if (view.length == MEWO_STRING_VIEW_SENTINEL) {
    return std::string(view.data);
  } else {
    return std::string(view.data, view.length);
  }
}

}  // namespace mewo::impl
