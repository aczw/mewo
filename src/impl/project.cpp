#include "impl/project.hpp"

#include "impl/project/pass.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace mewo::impl {
namespace {

constexpr std::string_view kShaderSource = R"(
struct Uniforms {
  resolution: vec2f,
  time: f32,
  delta_time: f32,
};

@group(0) @binding(0) var<uniform> mw: Uniforms;

@group(1) @binding(0) var prev_tex: texture_2d<f32>;
@group(1) @binding(1) var prev_smp: sampler;

// Coordinates for drawing an oversized triangle (acts as a fullscreen quad)
const kUvCoords = array(
  vec2f(0.0, 0.0), vec2f(2.0, 0.0), vec2f(0.0, 2.0)
);

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> @builtin(position) vec4f {
  let uv = kUvCoords[index];

  // Derive clip space positions from UV coords. This gives us
  // [0] = (-1.0, -1.0), [1] = (3.0, -1.0), [2] = (-1.0, 3.0)
  let position_xy = (2.0 * uv) - 1.0;

  return vec4f(position_xy, 0.0, 1.0);
}

@fragment
fn fs_main(@builtin(position) position: vec4f) -> @location(0) vec4f {
  let uv = position.xy / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}
)";

}  // namespace

std::unique_ptr<Project> Project::create() {
  std::vector<Pass> passes;
  passes.push_back(Pass(0, "Pass 1", std::string(kShaderSource)));

  // Constructor is private so can't use `std::make_unique`
  return std::unique_ptr<Project>(new Project(std::move(passes)));
}

}  // namespace mewo::impl
