#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace mewo {

enum class WgslType { kF32, kU32, kVec2f };

enum class UniformMember { kResolution, kTime, kDeltaTime, kFrameNumber, kFrameRate };

struct UniformField {
  UniformMember member;
  std::string_view name;
  WgslType type;
};

inline constexpr std::array kUniformFields = {
  UniformField{UniformMember::kResolution, "resolution", WgslType::kVec2f},
  UniformField{UniformMember::kTime, "time", WgslType::kF32},
  UniformField{UniformMember::kDeltaTime, "delta_time", WgslType::kF32},
  UniformField{UniformMember::kFrameNumber, "frame_number", WgslType::kU32},
  UniformField{UniformMember::kFrameRate, "frame_rate", WgslType::kU32},
};

constexpr bool validate_enum_field_order() {
  for (size_t idx = 0; idx < kUniformFields.size(); ++idx) {
    UniformMember field_member = kUniformFields[idx].member;
    UniformMember member = static_cast<UniformMember>(idx);

    if (field_member != member)
      return false;
  }

  return true;
}

static_assert(validate_enum_field_order(), "enum and fields must be in same order");

constexpr uint32_t align_of(WgslType type) {
  switch (type) {
    case WgslType::kF32: return 4;
    case WgslType::kU32: return 4;
    case WgslType::kVec2f: return 8;
  }
  std::unreachable();
}

constexpr uint32_t size_of(WgslType type) {
  switch (type) {
    case WgslType::kF32: return 4;
    case WgslType::kU32: return 4;
    case WgslType::kVec2f: return 8;
  }
  std::unreachable();
}

/**
 * WGSL has strict requirements about alignment and will add additional padding to offset
 * both member alignment and the overall struct padding. This calculates the smallest multiple
 * of `alignment` required that is >= `value`.
 */
constexpr uint32_t round_up(uint32_t value, uint32_t alignment) {
  return (value + alignment - 1) / alignment * alignment;
}

struct UniformsLayout {
  std::array<uint32_t, kUniformFields.size()> offsets;
  /** Refers to the number of bytes the overall struct has to take up. */
  uint32_t size;
};

constexpr UniformsLayout compute_uniforms_layout() {
  UniformsLayout layout = {};
  uint32_t current_end = 0;
  uint32_t current_struct_alignment = 0;

  for (size_t idx = 0; idx < kUniformFields.size(); ++idx) {
    WgslType field_type = kUniformFields[idx].type;
    uint32_t field_alignment = align_of(field_type);

    layout.offsets[idx] = round_up(current_end, field_alignment);
    current_end = layout.offsets[idx] + size_of(field_type);
    current_struct_alignment = std::max(field_alignment, current_struct_alignment);
  }

  layout.size = round_up(current_end, current_struct_alignment);
  return layout;
}

inline constexpr UniformsLayout kUniformsLayout = compute_uniforms_layout();

static_assert(kUniformsLayout.offsets[std::to_underlying(UniformMember::kResolution)] == 0);
static_assert(kUniformsLayout.offsets[std::to_underlying(UniformMember::kTime)] == 8);
static_assert(kUniformsLayout.offsets[std::to_underlying(UniformMember::kDeltaTime)] == 12);
static_assert(kUniformsLayout.offsets[std::to_underlying(UniformMember::kFrameNumber)] == 16);
static_assert(kUniformsLayout.offsets[std::to_underlying(UniformMember::kFrameRate)] == 20);

static_assert(kUniformsLayout.size == 24);

std::string generate_prelude_wgsl_uniforms();

}  // namespace mewo
