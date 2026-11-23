#include "impl/common/string.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace mewo::impl {

std::optional<std::string_view> to_std_string_view(mewo_string_view_t view) {
  if (view.data == nullptr && view.length != 0)
    return std::nullopt;

  if (view.length == 0)
    return std::string_view();

  if (view.length == MEWO_STRING_VIEW_SENTINEL) {
    return std::string_view(view.data);
  } else {
    return std::string_view(view.data, view.length);
  }
}

std::optional<std::string> to_string(mewo_string_view_t view) {
  auto string_view_opt = to_std_string_view(view);

  if (!string_view_opt)
    return std::nullopt;

  return std::string(string_view_opt.value());
}

mewo_string_view_t to_string_view(const std::string& string) {
  return {
    .data = string.data(),
    .length = string.length(),
  };
}

}  // namespace mewo::impl
