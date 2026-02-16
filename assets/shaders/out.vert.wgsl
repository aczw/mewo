const positions = array(
  vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
  vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
);

@vertex
fn main(@builtin(vertex_index) index: u32) -> @builtin(position) vec4f {
  return vec4f(positions[index], 0.0, 1.0);
}
