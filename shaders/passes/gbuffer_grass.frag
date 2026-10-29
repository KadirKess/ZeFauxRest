#version 450

in vec3 fragNormal;
in float fragHeight;
in float fragSeed;

layout(location = 0) out vec4 gColor;
layout(location = 1) out vec4 gNormal;

uniform vec3 grassBaseColor;
uniform vec3 grassTipColor;
uniform float grassHueVariation;
uniform float grassDryness;

vec3 rgb2hsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	vec3 col = mix(grassBaseColor, grassTipColor, fragHeight);

	// Per-blade hue jitter so the carpet isn't a single flat green.
	vec3 hsv = rgb2hsv(col);
	hsv.x = fract(hsv.x + (fragSeed - 0.5) * grassHueVariation);
	col = hsv2rgb(hsv);

	if (fragSeed < grassDryness)
		col = mix(col, vec3(0.62, 0.55, 0.23), 0.5);

	col *= mix(0.35, 1.0, smoothstep(0.0, 0.45, fragHeight));

	gColor = vec4(col, 1.0);

	// Foliage material-ID bit in gNormal.w - the deferred back-light reads it.
	gNormal = vec4(normalize(fragNormal), 1.0);
}
