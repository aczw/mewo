const u_DayLength = 15.0;
const u_Altitude = 0.0;

const base = vec3f(60, 100, 181) / 255.0;
const ambient = vec3f(148, 157, 161) / 255.0;
const nightBase = vec3f(2, 7, 15) / 255.0;
const nightAmbient = vec3f(3, 20, 42) / 255.0;
const sunset = vec3f(255, 222, 130) / 255.0;
const sunsetEdge = vec3f(245, 110, 37) / 255.0;
const sunsetPink = vec3f(148, 129, 111) / 255.0;
const sunsetPre = vec3f(191, 147, 153) / 255.0;
const sunsetPost = vec3f(235, 40, 0) / 255.0;
const density = 0.35;
const intensity = 2.0;
const horizonStrength = 0.5;
const horizonPowerSky = 8.0;
const horizonPowerSun = 1.5;
const horizonPowerSharp = 4.0;
const horizonOffset = -0.1;
const star1 = vec3f(208, 228, 245) / 255.0;
const star2 = vec3f(86, 143, 186) / 255.0;
const PI = 3.14159265;

fn random3(p: vec3f) -> vec3f {
    return fract(sin(vec3f(dot(p, vec3f(127.1, 311.7, 191.999)),
                          dot(p, vec3f(269.5, 183.3, 765.54)),
                          dot(p, vec3f(420.69, 631.2, 109.21))))
                 * 43758.5453);
}

fn fastWorley3D(p: vec3f) -> vec3f {
    let pointInt = floor(p);
    let pointFract = fract(p);
    var minDist = 1.0;
    var bestPoint = vec3f(0.0);

    for (var z = -1; z <= 1; z++) {
        for (var y = -1; y <= 1; y++) {
            for (var x = -1; x <= 1; x++) {
                let neighbor = vec3f(f32(x), f32(y), f32(z));
                let point = random3(pointInt + neighbor);
                let diff = neighbor + point - pointFract;
                let dist = length(diff);

                if (dist < minDist) {
                    minDist = dist;
                    bestPoint = pointInt + neighbor + point;

                    if (dist < 0.1) {
                        return bestPoint;
                    }
                }
            }
        }
    }

    return bestPoint;
}

fn lookAt(ro: vec3f, ta: vec3f, up: vec3f) -> mat3x3f
{
    let f = normalize(ta - ro);
    let r = normalize(cross(f, up));
    let u = cross(r, f);

    return mat3x3f(r, u, f);
}

fn myMod(x: f32, y: f32) -> f32 { return x - y * floor(x / y); }

@fragment
fn main(@builtin(position) pos: vec4f) -> @location(0) vec4f {
    var uvs = pos.xy / mw.resolution.xy;
    uvs.y = 1.0 - uvs.y;
    var p = uvs * 2.0 - 1.0;
    p.x *= (mw.resolution.x / mw.resolution.y);

    let lookDir = vec2f(mw.resolution.x * 0.37, mw.resolution.y * 0.6);
    let m = lookDir / mw.resolution.xy - 0.5;
    let yaw = m.x * 2.0 * PI;
    let pitch = clamp(m.y * PI, -PI * 0.5 + 0.001, PI * 0.5 - 0.001);
    let ro = vec3f(0.0);
    let forward = normalize(vec3(cos(pitch) * sin(yaw), sin(pitch), -cos(pitch) * cos(yaw)));
    let rd = normalize(lookAt(ro, forward, vec3(0.0, 1.0, 0.0)) * vec3(p * tan(radians(90.0) * 0.5), 1.0));

    let time = myMod(mw.time, u_DayLength) / u_DayLength;

    let lightX = cos(2.0 * PI * time);
    let lightY = sin(2.0 * PI * time);
    let lightZ = 0.0;
    let lightDir = vec3f(lightX, lightY, lightZ);

    let up = vec3f(0.0, 1.0, 0.0);
    let vertical = clamp(dot(rd, up), -1.0, 1.0);
    let exposure = exp2(vertical * 0.75 - 1.25);
    let baseGradient = exp(-(1.0 - pow(1.0 - max(vertical, 0.0), vertical)) / density);
    var sky: vec3f = base;
    sky *= baseGradient / (intensity * intensity);
    sky = sky / sqrt(sky * sky + 1.0) * exposure * (intensity * intensity);
    let sunMix = (vertical * 0.5 + 0.5) * clamp(1.0 - vertical, 0.0, 1.0) * pow(1.0 - vertical * 0.6, 3.0);
    let horizonMix = pow(1.0 - abs(vertical - horizonOffset), horizonPowerSky) * horizonStrength;
    let lightMix = (1.0 - (1.0 - sunMix) * (1.0 - horizonMix));
    let lightSky = pow(ambient, vec3f(3.0)) * baseGradient;
    sky = mix(sqrt(sky * (1.0 - lightMix)), sqrt(lightSky), lightMix);
    sky *= (sky * 5.0);

    let horizon = clamp(pow(1.0 - abs(vertical - horizonOffset), horizonPowerSun), 0.0, 1.0);
    let horizonSharp = clamp(pow(1.0 - abs(vertical - horizonOffset), horizonPowerSharp), 0.0, 1.0);
    let east = rd.x * 0.5 + 0.5;
    let west = 1.0 - east;
    let zenith = rd.y * 0.5 + 0.5;

    var night = mix(nightBase, nightAmbient, 1.0 - zenith);
    var space = night;
    night = mix(night, night * 2.0, horizonSharp);
    var dt = abs(time - 0.75);
    dt = min(dt, 1.0 - dt);
    var dayNight = 1.0 - (dt / 0.5);
    dayNight = smoothstep(0.0, 1.0, dayNight);
    sky = mix(sky, night, dayNight);

    let w = 0.12;
    let a = exp(-pow(time - 0.0, 2.0) / (2.0 * w * w));
    let b = exp(-pow(time - 1.0, 2.0) / (2.0 * w * w));
    let sunriseAmount = max(a, b);
    let sunsetAmount = exp(-pow(time - 0.5, 2.0) / (2.0 * w * w));

    let horizontal = cos(2.0 * PI * time);
    let sunCardinal = mix(east, west, 0.5 - 0.5 * horizontal);
    let sunHorizonDir = normalize(vec3f(horizontal * 20.0, -0.5, 0.0));

    var sunT = 1.2 * smoothstep(0.0, 1.0, 0.3 * (1.0 + dot(rd, sunHorizonDir)));
    sunT = mix(sunT, mix(sunT, 1.0, horizonSharp), clamp(abs(sunCardinal) + 0.1, 0.0, 1.0));
    sunT *= max(sunriseAmount, sunsetAmount);

    let sunBlend = mix(sunsetEdge * (sunT + 1.0), sunset, pow(sunT, 2.0));
    var sunSky = mix(sunsetPink, sunBlend, pow(sunT, 0.5));

    let sunPrePostBlend = pow(max(sunriseAmount, sunsetAmount), 3.0);
    if (time < 0.5) {
        sunSky = mix(sunsetPre, sunSky, sunPrePostBlend);
    } else {
        sunSky = mix(sunsetPost, sunSky, sunPrePostBlend);
    }

    let starScale = 15.0;
    let starMinSize = 0.0005;
    let starMaxSize = 0.002;
    let twinkleSpeed = 2.5;

    let fp = fastWorley3D(rd * starScale);
    var tanX = normalize(cross(up, fp));
    var tanY = normalize(cross(fp, tanX));
    var sp = vec2f(dot(rd, tanX), dot(rd, tanY));
    let r = random3(fp);

    var halfSize = clamp(starMaxSize * r.x, starMinSize, starMaxSize);
    let spaceStarHalfSize = halfSize;
    halfSize *= clamp((rd.y - horizonOffset), 0.25, 1.0);
    let dist = max(abs(sp.x), abs(sp.y)) - halfSize;
    let spaceDist = max(abs(sp.x), abs(sp.y)) - spaceStarHalfSize;

    var star = clamp(1.0 - dist / 0.001, 0.0, 1.0);
    var spaceStar = clamp(1.0 - spaceDist / 0.001, 0.0, 1.0);
    star *= (1.0 / 0.6) * (dayNight - 0.4);
    star = clamp(star - clamp(sunT * 2.0, 0.0, 1.0), 0.0, 1.0);

    var starHorizonRamp = clamp(rd.y - horizonOffset, 0.0, 1.0);
    starHorizonRamp = clamp(starHorizonRamp * 3.0, 0.0, 1.0);
    star = mix(0.0, star, starHorizonRamp);

    var twinkle = 0.5 * (1.0 + sin(twinkleSpeed * mw.time * r.z));
    twinkle = clamp(twinkle, 0.2, 1.0);
    star *= twinkle;
    spaceStar *= twinkle;

    var moonAmbient = dot(rd, -lightDir);
    moonAmbient = clamp(3.0 * moonAmbient - 2.0, 0.0, 1.0);
    moonAmbient = clamp(pow(moonAmbient, 2.0), 0.0, 1.0);
    moonAmbient *= (1.0 / 0.6) * (dayNight - 0.4);
    sky = mix(sky, mix(sky, nightAmbient, 0.9), moonAmbient);

    let starColor = mix(star1, star2, r.y);
    sky = mix(sky, starColor, clamp(star * 2.0 * (dayNight - 0.5), 0.0, 1.0));
    space = mix(space, starColor, spaceStar);

    var blendSkySun = mix(sky, sunSky, pow(sunT, 1.5));

    var sunRotation = 35.0;
    let sunSize = 0.07;
    let sunFeather = 0.001;

    var sunUp = vec3f(1, 0, 0);
    if (abs(lightDir.y) <= 0.99) {
        sunUp = up;
    }
    tanX = normalize(cross(sunUp, lightDir));
    tanY = normalize(cross(lightDir, tanX));
    sp = vec2f(dot(rd, tanX), dot(rd, tanY));

    sunRotation *= PI / 180.0;
    let s2 = sin(sunRotation);
    let c2 = cos(sunRotation);
    sp = vec2f(sp.x * c2 - sp.y * s2, sp.x * s2 + sp.y * c2);
    let sunDist = max(abs(sp.x), abs(sp.y)) - sunSize;
    let sunMoonShape = clamp(1.0 - sunDist / sunFeather, 0.0, 1.0);

    var sunGlow = dot(rd, lightDir);
    let sunGlowShrink = 10.0 - 2.0 * clamp(lightY, 0.0, 1.0);
    sunGlow = clamp(sunGlowShrink * sunGlow - (sunGlowShrink - 1.0), 0.0, 1.0);
    sunGlow = clamp(pow(sunGlow, 10.0), 0.0, 1.0);
    
    var moonGlow = dot(rd, -lightDir);
    moonGlow = clamp(10.0 * moonGlow - 9.0, 0.0, 1.0);
    moonGlow = clamp(pow(moonGlow, 10.0), 0.0, 1.0);

    let sunGlowMix = 0.5 + 0.2 * clamp(lightY, 0.0, 1.0);
    blendSkySun = mix(blendSkySun, mix(blendSkySun, vec3f(1.0), sunGlowMix), sunGlow);
    blendSkySun = mix(blendSkySun, mix(blendSkySun, star1, 0.3), moonGlow);
    blendSkySun = mix(blendSkySun, vec3f(1.0), sunMoonShape);

    space = mix(space, mix(space, vec3f(1.0), sunGlowMix), sunGlow);
    space = mix(space, mix(space, star1, 0.3), moonGlow);
    space = mix(space, vec3f(1.0), sunMoonShape);

    let spaceStart = 256.0;
    let spaceEnd = 512.0;
    let spaceT = clamp(u_Altitude - spaceStart, 0.0, (spaceEnd - spaceStart)) / (spaceEnd - spaceStart);

    let finalColor = mix(blendSkySun, space, spaceT);
    return vec4f(finalColor, 1.0);
}
