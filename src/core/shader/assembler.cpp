#include "core/shader/assembler.hpp"

#include "core/shader/uniforms.hpp"

#include <string>
#include <string_view>

namespace mewo {
namespace {

constexpr std::string_view kPreludeSource = R"(
@group(1) @binding(0) var prev_tex: texture_2d<f32>;
@group(1) @binding(1) var prev_smp: sampler;

var<private> mw: mw_Uniforms;

// Coordinates for drawing an oversized triangle (acts as a fullscreen quad)
const mw_kUvCoords = array(vec2f(0.0, 0.0), vec2f(2.0, 0.0), vec2f(0.0, 2.0));

@vertex
fn mw_vs_main(@builtin(vertex_index) index: u32) -> @builtin(position) vec4f {
  let uv = mw_kUvCoords[index];

  // Derive clip space positions from UV coords. This gives us
  // [0] = (-1.0, -1.0), [1] = (3.0, -1.0), [2] = (-1.0, 3.0)
  let position_xy = (2.0 * uv) - 1.0;

  return vec4f(position_xy, 0.0, 1.0);
}

@fragment
fn mw_fs_main(@builtin(position) position: vec4f) -> @location(0) vec4f {
  mw = mw_uniforms;
  return main(position.xy);
}
)";

}  // namespace

std::string assemble_pass_source(std::string_view pass_source, std::string_view common_source) {
  std::string code(pass_source);
  code += '\n';
  code += common_source;
  code += '\n';
  code += generate_wgsl_uniforms();
  code += kPreludeSource;
  return code;
}

}  // namespace mewo
