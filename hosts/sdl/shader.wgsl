fn main(frag_coord: vec2f) -> vec4f {
    var color: vec4f = helper();
    color.y = sin(mw.time) * 0.5 + 0.5;
    color.a = 1.0;

    return color;
}
