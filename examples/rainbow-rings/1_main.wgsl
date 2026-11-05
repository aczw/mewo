fn main(frag_coord: vec2f) -> vec4f {
    let uv = frag_coord / mw.resolution;
    let point = (frag_coord - 0.5 * mw.resolution) / mw.resolution.y;

    let rings = 0.5 + 0.5 * cos(40.0 * length(point) - 3.0 * mw.time);
    let color = palette(uv.x + 0.1 * mw.time) * rings;

    return vec4f(color, 1.0);
}
