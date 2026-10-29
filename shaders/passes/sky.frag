#version 450

#define SKY_GRADIENT 0
#define SKY_HOSEK_WILKIE 1

#ifndef SKY_MODEL
#	define SKY_MODEL SKY_GRADIENT
#endif

#include "../shared/common.glsl"

in vec2 vNdc;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gDepth;

uniform mat4 invViewProj;

#if SKY_MODEL == SKY_GRADIENT
uniform vec3 horizonColor;
uniform vec3 zenithColor;
#endif

uniform vec3 sunDiscColor;
uniform float sunDiscCos; // cos(angular radius)

void main()
{
	float depth = texelFetch(gDepth, ivec2(gl_FragCoord.xy), 0).r;
	if (depth < 1.0)
		discard;

	vec4 nearH = invViewProj * vec4(vNdc, -1.0, 1.0);
	vec4 farH = invViewProj * vec4(vNdc, 1.0, 1.0);
	vec3 viewDir = normalize(farH.xyz / farH.w - nearH.xyz / nearH.w);

	vec3 color;

#if SKY_MODEL == SKY_GRADIENT
	float t = clamp(viewDir.y, 0.0, 1.0);
	color = mix(horizonColor, zenithColor, t);

	if (dot(viewDir, sunDir) > sunDiscCos)
		color = sunDiscColor;

#elif SKY_MODEL == SKY_HOSEK_WILKIE
	color = sampleSky(viewDir);

	// sunDiscCos stores cos(half-angle), acos recovers the angle for the soft
	// edge
	float cosGamma = dot(viewDir, sunDir);
	float discAngle = acos(clamp(sunDiscCos, -1.0, 1.0));
	float softEdge = 0.004;
	float disc = smoothstep(cos(discAngle + softEdge), sunDiscCos, cosGamma);
	color += sunDiscColor * disc;
#endif

	outColor = vec4(color, 1.0);
}
