const PI: f32 = 3.14159;

fn pp(x: f32) -> f32 {
    return 0.5 * x + 0.5;
}

@fragment
fn main(@builtin(position) position: vec4f) -> @location(0) vec4f {
    return vec4f(pp(cos(mw.time)), pp(sin(mw.time)), pp(sin(mw.time + PI)), 1.0);
}
