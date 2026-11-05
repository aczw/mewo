fn main(frag_coord: vec2f) -> vec4f {
    let uv = frag_coord / mw.resolution;
    let color = textureSample(prev_tex, prev_smp, uv).rgb;

    let vignette = smoothstep(0.8, 0.3, distance(uv, vec2f(0.5)));

    return vec4f(color * vignette, 1.0);
}
