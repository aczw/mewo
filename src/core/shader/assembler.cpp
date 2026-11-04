#include "core/shader/assembler.hpp"

#include "core/shader/uniforms.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace mewo {
namespace {

constexpr std::string_view kPreludeEntrySource =
  R"wgsl(@group(1) @binding(0) var prev_tex: texture_2d<f32>;
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
})wgsl";

}  // namespace

SegmentMap::SegmentMap(Range pass, Range common, Range prelude)
    : pass_(pass), common_(common), prelude_(prelude) {
  assert(pass_.start == 1);
  assert(common_.start == pass_.end + 1);
  assert(prelude_.start == common_.end + 1);
}

std::optional<RelativeLine> SegmentMap::find_relative_line(uint32_t line_number) const {
  if (line_number == 0 || line_number > prelude_.end)
    return std::nullopt;

  if (line_number <= pass_.end)
    return RelativeLine{.kind = RelativeLine::Kind::kPass, .number = line_number};

  if (line_number <= common_.end) {
    return RelativeLine{
      .kind = RelativeLine::Kind::kCommon, .number = line_number - common_.start + 1
    };
  }

  return RelativeLine{
    .kind = RelativeLine::Kind::kPrelude,
    .number = line_number - prelude_.start + 1,
  };
}

AssembledSource assemble_pass_source(std::string_view pass_source, std::string_view common_source) {
  std::string code;
  uint32_t next_line = 1;

  auto append_segment = [&code, &next_line](std::string_view piece) -> SegmentMap::Range {
    uint32_t start = next_line;
    code += piece;
    code += '\n';
    next_line += static_cast<uint32_t>(std::ranges::count(piece, '\n') + 1);

    return {.start = start, .end = next_line - 1};
  };

  auto pass = append_segment(pass_source);
  auto common = append_segment(common_source);
  auto prelude =
    append_segment(generate_prelude_wgsl_uniforms() + std::string(kPreludeEntrySource));

  return {
    .code = std::move(code),
    .segments = SegmentMap(pass, common, prelude),
  };
}

}  // namespace mewo
