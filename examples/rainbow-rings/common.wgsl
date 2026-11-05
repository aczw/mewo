const kPi = 3.14159;

fn palette(t: f32) -> vec3f {
    return 0.5 + 0.5 * cos(2 * kPi * (t + vec3f(0.0, 0.33, 0.67)));
}
