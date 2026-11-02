#pragma once

#include "core/shader/uniforms.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace mewo {

class UniformData {
 public:
  UniformData() = default;

  UniformData(UniformData&&) = default;
  UniformData& operator=(UniformData&&) = default;
  UniformData(const UniformData&) = default;
  UniformData& operator=(const UniformData&) = default;

  void set(UniformMember member, float value);
  void set(UniformMember member, uint32_t value);
  void set(UniformMember member, std::array<float, 2> value);

  std::span<const std::byte> bytes() const { return bytes_; }

 private:
  std::array<std::byte, kUniformsLayout.size> bytes_ = {};
};

}  // namespace mewo
