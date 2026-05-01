// Originally created by Inigo Quilez
// Original Shadertoy: https://www.shadertoy.com/view/Mss3zH

fn distanceToSegment(a: vec2f, b: vec2f, p: vec2f) -> f32 {
	let pa: vec2f = p - a;
	let ba: vec2f = b - a;
	let h: f32 = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);

	return length(pa - ba * h);
}

@fragment
fn main(@builtin(position) position: vec4f) -> @location(0) vec4f {
	var pos = position;
	pos.y = mw.resolution.y - pos.y;

	let p: vec2f = pos.xy / mw.resolution.x;
    let cen: vec2f = 0.5 * mw.resolution.xy / mw.resolution.x;
    let m: vec4f = mw.mouse / mw.resolution.x;

	var col = vec3f(0.0);
	if m.z > 0.0 { // Button is down
		let d: f32 = distanceToSegment(m.xy, abs(m.zw), p);
        col = mix(col, vec3f(1.0, 1.0, 0.0), 1.0 - smoothstep(.004, 0.008, d));
	}
	if m.w > 0.0 { // Button click
        col = mix(col, vec3f(1.0), 1.0 - smoothstep(0.1, 0.105, length(p - cen)));
    }

	col = mix(col, vec3f(1.0, 0.0, 0.0), 1.0 - smoothstep(0.03, 0.035, length(p - m.xy)));
    col = mix(col, vec3f(0.0, 0.0, 1.0), 1.0 - smoothstep(0.03, 0.035, length(p - abs(m.zw))));

    return vec4f(col, 1.0);
}
