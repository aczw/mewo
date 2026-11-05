const kRadius = 30;
const kSpacing = 2.0;

fn main(frag_coord: vec2f) -> vec4f {
    let texel_size = 1.0 / mw.resolution;
    let uv = frag_coord * texel_size;
    let sigma = f32(kRadius) / 3.0;

    var sum = vec3f(0.0);
    var total_weight = 0.0;

    for(var y = -kRadius; y <= kRadius; y++) {
        for(var x = -kRadius; x <= kRadius; x++) {
            let offset = vec2f(f32(x), f32(y));
            let weight = exp(-dot(offset, offset) / (2.0 * sigma * sigma));
            let sample_uv = uv + offset * kSpacing * texel_size;

            sum += textureSampleLevel(prev_tex, prev_smp, sample_uv, 0.0).rgb * weight;
            total_weight += weight;
        }
    }

    return vec4f(sum / total_weight, 1.0);
}
