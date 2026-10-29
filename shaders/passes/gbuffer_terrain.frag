#version 450

#define NORMAL_MODE_CENTRAL_DIFF_2TAP 0

#ifndef NORMAL_MODE
#	define NORMAL_MODE NORMAL_MODE_CENTRAL_DIFF_2TAP
#endif

in vec2 fragUV;

layout(location = 0) out vec4 gColor;
layout(location = 1) out vec4 gNormal;

uniform sampler2D heightMap;
uniform float texelSize;
uniform float worldStep;
uniform float heightScale;
uniform vec3 terrainColor;
uniform sampler2D coverageMask;

void main()
{
	float height = texture(heightMap, fragUV).r;

	vec3 normal;
#if NORMAL_MODE == NORMAL_MODE_CENTRAL_DIFF_2TAP
	float hUp = texture(heightMap, fragUV + vec2(0.0, texelSize)).r;
	float hDown = texture(heightMap, fragUV - vec2(0.0, texelSize)).r;
	float hLeft = texture(heightMap, fragUV - vec2(texelSize, 0.0)).r;
	float hRight = texture(heightMap, fragUV + vec2(texelSize, 0.0)).r;

	float dX = (hRight - hLeft) * heightScale;
	float dZ = (hUp - hDown) * heightScale;

	// dX/dZ are world-space height deltas, so the Y term must be the matching
	// world-space horizontal span (2 * worldStep), not the UV-space texel size.
	normal = normalize(vec3(-dX, 2.0 * worldStep, -dZ));
#endif

	// Modulate the single base tone instead of carrying separate look values so
	// valleys read darker, steep faces brighter where dirt is exposed.
	vec3 valley = terrainColor * 0.65;
	vec3 albedo = mix(valley, terrainColor, smoothstep(0.0, 0.5, height));
	float slope = 1.0 - normal.y;
	albedo = mix(albedo, terrainColor * 1.35, smoothstep(0.2, 0.55, slope));

	// Sink the soil into shadow where grass roots densely - sells rooted
	// blades.
	float coverage = texture(coverageMask, fragUV).r;
	albedo *= mix(0.7, 1.0, 1.0 - coverage);

	gColor = vec4(albedo, 1.0);
	gNormal = vec4(normal, 0.0);
}
