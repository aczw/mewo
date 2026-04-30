// Originally created by inigo quilez - iq/2016
// Original Shadertoy: https://www.shadertoy.com/view/lsKGWV
// License: CC-BY-NC-SA-3.0

// Font by P_Malin (https://www.shadertoy.com/view/4sf3RN)
const FONT_LUT = array<u32, 10>(0x75557, 0x22222, 0x74717, 0x74747, 0x11574, 0x71747, 0x71757, 0x74444, 0x75757, 0x75747);

fn glsl_mod(a: f32, b: f32) -> f32 {
    return a - b * floor(a / b);
}

fn ud_round_box(p: vec2f, b: vec2f, r: f32) -> f32 {
    return length(max(abs(p) - b, vec2f(0.0))) - r;
}

fn sd_box(p: vec2f, b: vec2f) -> f32 {
    let d: vec2f = abs(p) - b;
    return min(max(d.x, d.y), 0.0) + length(max(d, vec2f(0.0)));
}

// Digit drawing function by P_Malin (https://www.shadertoy.com/view/4sf3RN)
fn sample_digit(n: i32, uv: vec2f) -> f32
{
	if (uv.x  < 0.0) { return 0.0; }
	if (uv.y  < 0.0) { return 0.0; }
	if (uv.x >= 1.0) { return 0.0; }
	if (uv.y >= 1.0) { return 0.0; }

    let p = vec2i(floor(uv * vec2f(4.0, 5.0)));
    return f32((FONT_LUT[n] >> u32(p.x + p.y * 4)) & 1);
}

fn print_int(uv: vec2f, value: f32) -> f32
{
	var res = 0.0;
	let maxDigits = 1.0 + ceil(log2(value) / log2(10.0));
	let digitID = floor(uv.x);

	if(digitID > 0.0 && digitID < maxDigits) {
        let digitVa: f32 = glsl_mod(floor(value / pow(10.0, maxDigits - 1.0 - digitID)), 10.0);
        res = sample_digit(i32(digitVa), vec2(fract(uv.x), uv.y));
	}

	return res;
}

@fragment
fn main(@builtin(position) position: vec4f) -> @location(0) vec4f {
	let px = 1.0 / mw.resolution.y;
    let uv = vec2f(position.x * px, 1.0 - position.y * px);

    // Top: FPS (mw.frame_rate) as an integer
    var col: f32 = print_int((uv - vec2f(0.2, 0.75)) * 10.0, f32(mw.frame_rate));

    // Middle: 1.0 / mw.delta_time as an integer
    col += print_int((uv - vec2f(0.2, 0.5)) * 10.0, floor(1.0 / mw.delta_time + 0.5));

    // Nottom: odd/even frame (mw.frame_number) box
    col += (1.0 - smoothstep(0.0, px, ud_round_box(uv - vec2f(0.4,0.2), vec2f(0.1), 0.05))) *
            step(abs(fract(0.5 * f32(mw.frame_number)) - 0.5), 0.1);

    // mw.delta_time as vertical bar
    col += (1.0 - smoothstep(0.0, px, sd_box(uv - vec2f(0.1, 0.0), vec2f(0.02, 60.0 * mw.delta_time))));

	return vec4f(vec3f(col), 1.0);
}
