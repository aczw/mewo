struct Unif {
  time: f32,
  resolution: vec2f,
};

@group(0) @binding(0)
var<uniform> mw: Unif;

@fragment
fn main(@builtin(position) position: vec4f) -> @location(0) vec4f {
  let uv = position.xy / mw.resolution;
  // return vec4f(cos(mw.time), 1.0, sin(mw.time), 1.0);
  return vec4f(uv, 0.0, 1.0);
}
