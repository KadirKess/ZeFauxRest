#version 450

in vec2 vUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D source;
uniform vec2 srcTexel;
uniform float weight;

// 9-tap tent filter. Output is additively blended, so mips combine as a
// weighted sum front-loaded weights drive a tight halo.
void main()
{
	vec2 t = srcTexel;

	vec3 sum = texture(source, vUv).rgb * 4.0;
	sum += texture(source, vUv + t * vec2(1.0, 0.0)).rgb * 2.0;
	sum += texture(source, vUv + t * vec2(-1.0, 0.0)).rgb * 2.0;
	sum += texture(source, vUv + t * vec2(0.0, 1.0)).rgb * 2.0;
	sum += texture(source, vUv + t * vec2(0.0, -1.0)).rgb * 2.0;
	sum += texture(source, vUv + t * vec2(1.0, 1.0)).rgb;
	sum += texture(source, vUv + t * vec2(-1.0, 1.0)).rgb;
	sum += texture(source, vUv + t * vec2(1.0, -1.0)).rgb;
	sum += texture(source, vUv + t * vec2(-1.0, -1.0)).rgb;
	sum /= 16.0;

	outColor = vec4(sum * weight, 1.0);
}
