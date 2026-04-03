@fragment
fn main(@builtin(position) position: vec4f) -> @location(0) vec4f {
    return vec4f(position.xy / mw.resolution.xy, 0.0, 1.0);
}
