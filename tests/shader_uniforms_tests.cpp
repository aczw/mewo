#include "core/shader/uniforms.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("uniform WGSL matches the uniform field table", "[shader][uniforms]") {
  const std::string expected = R"wgsl(struct mw_Uniforms {
  resolution: vec2f,
  time: f32,
  delta_time: f32,
  frame_number: u32,
  frame_rate: u32,
};
@group(0) @binding(0) var<uniform> mw_uniforms: mw_Uniforms;
)wgsl";

  REQUIRE(mewo::generate_prelude_wgsl_uniforms() == expected);
}
