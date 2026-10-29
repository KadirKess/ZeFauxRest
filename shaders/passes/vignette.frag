#version 450

in vec2 vUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D tonemapped;
uniform float vignetteStrength;

void main()
{
	vec3 color = texture(tonemapped, vUv).rgb;

	float d = distance(vUv, vec2(0.5)) * 1.41421356; // 1.0 at the corners
	float v = 1.0 - vignetteStrength * smoothstep(0.5, 1.0, d);
	color *= v;

	// default framebuffer is not sRGB-capable so we encode manually
	vec3 srgb = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));
	outColor = vec4(srgb, 1.0);
}
