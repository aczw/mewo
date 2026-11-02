#include "core/shader/uniform_data.hpp"
#include "core/shader/uniforms.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <utility>

namespace {

uint32_t offset_of(mewo::UniformMember member) {
  return mewo::kUniformsLayout.offsets[static_cast<size_t>(std::to_underlying(member))];
}

template <typename T>
T read_at(std::span<const std::byte> bytes, uint32_t offset) {
  T value;
  std::memcpy(&value, bytes.data() + offset, sizeof(T));
  return value;
}

bool all_zero(std::span<const std::byte> bytes, uint32_t offset, size_t count) {
  std::span<const std::byte> range = bytes.subspan(offset, count);
  return std::ranges::all_of(range, [](std::byte b) { return b == std::byte{0}; });
}

}  // namespace

TEST_CASE("uniform data matches the uniform layout", "[shader][uniforms]") {
  using mewo::UniformMember;

  mewo::UniformData data;

  SECTION("is exactly the size of the uniform struct") {
    REQUIRE(data.bytes().size() == mewo::kUniformsLayout.size);
  }

  SECTION("starts zeroed") { REQUIRE(all_zero(data.bytes(), 0, data.bytes().size())); }

  SECTION("writes an f32 at its offset") {
    data.set(UniformMember::kTime, 1.5f);
    REQUIRE(read_at<float>(data.bytes(), offset_of(UniformMember::kTime)) == 1.5f);
  }

  SECTION("writes a u32 at its offset") {
    data.set(UniformMember::kFrameNumber, uint32_t{42});
    REQUIRE(read_at<uint32_t>(data.bytes(), offset_of(UniformMember::kFrameNumber)) == 42);
  }

  SECTION("writes a vec2f at its offset") {
    data.set(UniformMember::kResolution, std::array<float, 2>{1280.0f, 720.0f});

    auto resolution =
      read_at<std::array<float, 2>>(data.bytes(), offset_of(UniformMember::kResolution));
    REQUIRE(resolution[0] == 1280.0f);
    REQUIRE(resolution[1] == 720.0f);
  }

  SECTION("leaves other members untouched") {
    data.set(UniformMember::kTime, 1.5f);

    REQUIRE(all_zero(data.bytes(), offset_of(UniformMember::kResolution), 8));
    REQUIRE(all_zero(data.bytes(), offset_of(UniformMember::kDeltaTime), 4));
    REQUIRE(all_zero(data.bytes(), offset_of(UniformMember::kFrameNumber), 4));
    REQUIRE(all_zero(data.bytes(), offset_of(UniformMember::kFrameRate), 4));
  }

  SECTION("writes every member without overlap") {
    data.set(UniformMember::kResolution, std::array<float, 2>{1280.0f, 720.0f});
    data.set(UniformMember::kTime, 1.5f);
    data.set(UniformMember::kDeltaTime, 0.25f);
    data.set(UniformMember::kFrameNumber, uint32_t{42});
    data.set(UniformMember::kFrameRate, uint32_t{60});

    auto resolution =
      read_at<std::array<float, 2>>(data.bytes(), offset_of(UniformMember::kResolution));
    REQUIRE(resolution[0] == 1280.0f);
    REQUIRE(resolution[1] == 720.0f);
    REQUIRE(read_at<float>(data.bytes(), offset_of(UniformMember::kTime)) == 1.5f);
    REQUIRE(read_at<float>(data.bytes(), offset_of(UniformMember::kDeltaTime)) == 0.25f);
    REQUIRE(read_at<uint32_t>(data.bytes(), offset_of(UniformMember::kFrameNumber)) == 42);
    REQUIRE(read_at<uint32_t>(data.bytes(), offset_of(UniformMember::kFrameRate)) == 60);
  }
}
