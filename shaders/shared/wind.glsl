#ifndef WIND_GLSL
#define WIND_GLSL

// grass and tree foliage read the same UBO so they sway in sync

struct WindParams
{
	float time;
	vec2 direction;
	float strength;
	float gustFrequency;
};

layout(binding = 0) uniform WindUniforms
{
	float uWindTime;
	vec2 uWindDirection;
	float uWindStrength;
	float uWindGustFrequency;
};

WindParams windParams()
{
	return WindParams(uWindTime, uWindDirection, uWindStrength,
					  uWindGustFrequency);
}

// callers scale by normalized height so the tip bends more than the base
vec3 applyWind(vec3 worldPos, WindParams p)
{
	float phase = dot(worldPos.xz, p.direction) * 0.15 + p.time;
	float gust = 0.5 + 0.5 * sin(p.time * p.gustFrequency);
	float bend = sin(phase) * p.strength * gust;
	return vec3(p.direction.x, 0.0, p.direction.y) * bend;
}

#endif
