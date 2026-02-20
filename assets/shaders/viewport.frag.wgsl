@group(0) @binding(0)
var<uniform> mw_time: f32;

@fragment
fn main() -> @location(0) vec4f {
  return vec4f(cos(mw_time), 1.0, sin(mw_time), 1.0);
}
