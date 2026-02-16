#pragma once

#include "exception.hpp"
#include "query.hpp"

#include <string_view>
#include <type_traits>
#include <utility>

namespace mewo::utility {

/// In debug builds, throws an exception about an unhandled enum case.
/// Otherwise, it calls `std::unreachable()` which invokes UB.
template <class Enum>
  requires std::is_enum_v<Enum>
inline void enum_unreachable(std::string_view enum_name, Enum value)
{
  if constexpr (query::is_debug()) {
    throw Exception("Unhandled {} case: {}", enum_name, std::to_underlying(value));
  } else {
    std::unreachable();
  }
}

}
