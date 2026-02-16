#pragma once

namespace mewo {

class AspectRatio {
  public:
  enum class Preset : int { e1_1, e2_1, e3_2, e16_9 };

  static float get_inverse_value(Preset preset);
};

}
