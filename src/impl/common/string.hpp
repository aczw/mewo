#pragma once

#include <mewo/mewo.h>

#include <optional>
#include <string>

namespace mewo::impl {

std::optional<std::string> to_string(mewo_string_view_t view);

mewo_string_view_t to_string_view(const std::string& string);

}  // namespace mewo::impl
