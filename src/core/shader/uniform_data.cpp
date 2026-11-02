#include "core/shader/uniform_data.hpp"

#include "core/shader/uniforms.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>
#include <utility>

namespace mewo {
namespace {

template <typename T>
void write_at(std::span<std::byte> bytes, uint32_t offset, const T& value) {
  static_assert(std::is_trivially_copyable_v<T>);
  std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

}  // namespace

void UniformData::set(UniformMember member, float value) {
  size_t idx = static_cast<size_t>(std::to_underlying(member));
  assert(kUniformFields[idx].type == WgslType::kF32 && "field is not f32");
  write_at(bytes_, kUniformsLayout.offsets[idx], value);
}

void UniformData::set(UniformMember member, uint32_t value) {
  size_t idx = static_cast<size_t>(std::to_underlying(member));
  assert(kUniformFields[idx].type == WgslType::kU32 && "field is not u32");
  write_at(bytes_, kUniformsLayout.offsets[idx], value);
}

void UniformData::set(UniformMember member, std::array<float, 2> value) {
  size_t idx = static_cast<size_t>(std::to_underlying(member));
  assert(kUniformFields[idx].type == WgslType::kVec2f && "field is not vec2f");
  static_assert(sizeof(std::array<float, 2>) == 8);
  write_at(bytes_, kUniformsLayout.offsets[idx], value);
}

}  // namespace mewo
