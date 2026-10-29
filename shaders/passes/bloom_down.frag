#version 450

in vec2 vUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D source;
uniform vec2 srcTexel;

// 13-tap downsample (Jimenez, "Next Generation Post Processing in Call of Duty,
// Advanced Warfare", SIGGRAPH 2014). Bilinear sampling gives 36 effective taps.
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

	vec3 result = e * 0.125;
	result += (a + c + g + i) * 0.03125;
	result += (b + d + f + h) * 0.0625;
	result += (j + k + l + m) * 0.125;

	outColor = vec4(result, 1.0);
}
