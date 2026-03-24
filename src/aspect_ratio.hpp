#pragma once

#include "utility.hpp"

namespace mewo {

class AspectRatio {
  public:
  enum class Preset : int { e1_1, e2_1, e3_2, e16_9 };

  static float get_inverse_value(Preset preset)
  {
    switch (preset) {
      // clang-format off
  case Preset::e1_1: return 1.f;
  case Preset::e2_1: return 1.f / 2.f;
  case Preset::e3_2: return 2.f / 3.f;
  case Preset::e16_9: return 9.f / 16.f;
      // clang-format on

    default:
      utility::enum_unreachable("AspectRatio::Preset", preset);
    }
  }
};

}
