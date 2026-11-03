fn main(frag_coord: vec2f) -> vec4f {
    return vec4f(0.0, sin(mw.time) * 0.5 + 0.5, 1.0, 1.0);
}
