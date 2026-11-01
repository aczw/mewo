#pragma once

#include <cstdint>

namespace mewo::impl {

struct Dimensions {
  uint32_t width;
  uint32_t height;

  bool operator==(const Dimensions&) const = default;
};

}  // namespace mewo::impl
