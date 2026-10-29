#version 450

#include "../shared/luma.glsl"

in vec2 vUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D tonemapped;

uniform vec3 gradeLift;
uniform vec3 gradeGamma;
uniform vec3 gradeGain;
uniform vec3 highlightTint;
uniform vec3 shadowTint;

void main()
{
	vec3 c = texture(tonemapped, vUv).rgb;

	// Runs after Hable, before the sRGB encode, so it shapes the display image.
	vec3 graded = pow(c * gradeGain + gradeLift, 1.0 / gradeGamma);
	graded = mix(graded, graded * highlightTint, luma(graded));
	graded += shadowTint * (1.0 - luma(graded));

	outColor = vec4(graded, 1.0);
}
