#include "aspect_ratio.hpp"

#include "utility.hpp"

namespace mewo {

/// Guarantee these values are computed at compile-time.
namespace inverse_ratio {

constexpr float VALUE_1_1 = 1.f / 1.f;
constexpr float VALUE_2_1 = 1.f / 2.f;
constexpr float VALUE_3_2 = 2.f / 3.f;
constexpr float VALUE_16_9 = 9.f / 16.f;

}

float AspectRatio::get_inverse_value(Preset preset)
{
  switch (preset) {
    // clang-format off
  case Preset::e1_1: return inverse_ratio::VALUE_1_1;
  case Preset::e2_1: return inverse_ratio::VALUE_2_1;
  case Preset::e3_2: return inverse_ratio::VALUE_3_2;
  case Preset::e16_9: return inverse_ratio::VALUE_16_9;
    // clang-format on

  default:
    utility::enum_unreachable("AspectRatio::Preset", preset);
  }
}

}
