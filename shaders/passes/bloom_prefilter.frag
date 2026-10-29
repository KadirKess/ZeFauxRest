#version 450

in vec2 vUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D source;
uniform vec2 srcTexel;
uniform float threshold;

void main()
{
	vec2 t = srcTexel;

	vec3 a = texture(source, vUv + t * vec2(-2.0, 2.0)).rgb;
	vec3 b = texture(source, vUv + t * vec2(0.0, 2.0)).rgb;
	vec3 c = texture(source, vUv + t * vec2(2.0, 2.0)).rgb;
	vec3 d = texture(source, vUv + t * vec2(-2.0, 0.0)).rgb;
	vec3 e = texture(source, vUv).rgb;
	vec3 f = texture(source, vUv + t * vec2(2.0, 0.0)).rgb;
	vec3 g = texture(source, vUv + t * vec2(-2.0, -2.0)).rgb;
	vec3 h = texture(source, vUv + t * vec2(0.0, -2.0)).rgb;
	vec3 i = texture(source, vUv + t * vec2(2.0, -2.0)).rgb;
	vec3 j = texture(source, vUv + t * vec2(-1.0, 1.0)).rgb;
	vec3 k = texture(source, vUv + t * vec2(1.0, 1.0)).rgb;
	vec3 l = texture(source, vUv + t * vec2(-1.0, -1.0)).rgb;
	vec3 m = texture(source, vUv + t * vec2(1.0, -1.0)).rgb;

	vec3 box0 = (j + k + l + m) * 0.25;
	vec3 box1 = (a + b + d + e) * 0.25;
	vec3 box2 = (b + c + e + f) * 0.25;
	vec3 box3 = (d + e + g + h) * 0.25;
	vec3 box4 = (e + f + h + i) * 0.25;

	vec3 result = box0 * 0.5 + (box1 + box2 + box3 + box4) * 0.125;

	// Soft-knee threshold, keep energy above `threshold`, ramp the knee so the
	// halo edge doesn't hard-clip.
	float br = max(result.r, max(result.g, result.b));
	float knee = threshold * 0.5;
	float soft = clamp(br - threshold + knee, 0.0, 2.0 * knee);
	soft = soft * soft / (4.0 * knee + 1e-5);
	float contrib = max(soft, br - threshold) / max(br, 1e-5);

	outColor = vec4(result * contrib, 1.0);
}
