#include "aspect_ratio.hpp"

#include "util/enum_unreachable.hpp"

namespace mewo {

float AspectRatio::get_inverse_value(Preset preset) {
  switch (preset) {
    case Preset::e1_1: return 1.f;
    case Preset::e2_1: return 1.f / 2.f;
    case Preset::e3_2: return 2.f / 3.f;
    case Preset::e16_9: return 9.f / 16.f;

    default: util::enum_unreachable("AspectRatio::Preset", preset);
  }
}

}  // namespace mewo
