#version 450

#include "../shared/common.glsl"

in vec2 vUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D litScene;
uniform sampler2D cloudRGBA;
uniform sampler2D godrayRGB;
uniform sampler2D gDepth;
uniform sampler2D minDepthHalf; // R32F

uniform mat4 invViewProj;
uniform vec3 camPos;

// Aerial perspective, distance haze toward the sky color, height-faded so
// eye-level air ramps hard while upward views stay clear.
uniform float apStrength;
uniform float apStart;
uniform float apFalloff;
uniform float apCanopyFade;

vec3 worldPos(vec2 uv, float depth)
{
	vec2 ndc = uv * 2.0 - 1.0;
	vec4 wp = invViewProj * vec4(ndc, depth * 2.0 - 1.0, 1.0);
	return wp.xyz / wp.w;
}

// World-space distance from the camera, linearized so the silhouette test
// compares meters, not nonlinear window depth.
float viewDist(vec2 uv, float depth)
{
	return distance(camPos, worldPos(uv, depth));
}

void main()
{
	vec3 lit = texture(litScene, vUv).rgb;

	// Upsample the half-res clouds and godrays, favouring samples at a similar
	// depth so their edges line up with the full-res scene instead of bleeding
	// a halo across it.
	vec2 hsize = vec2(textureSize(minDepthHalf, 0));
	vec2 hp = vUv * hsize - 0.5;
	ivec2 base = ivec2(floor(hp));
	vec2 f = fract(hp);

	float sceneDepth = texture(gDepth, vUv).r;
	float fullDist = viewDist(vUv, sceneDepth);

	ivec2 offs[4] =
		ivec2[4](ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1));
	float bw[4] = float[4]((1.0 - f.x) * (1.0 - f.y), f.x * (1.0 - f.y),
						   (1.0 - f.x) * f.y, f.x * f.y);

	vec4 cloudAccum = vec4(0.0);
	vec3 godrayAccum = vec3(0.0);
	// Unclamped bilinear godray, for the silhouette fallback below.
	vec3 godrayBilinear = vec3(0.0);
	float wsum = 0.0;
	for (int i = 0; i < 4; i++)
	{
		ivec2 c = clamp(base + offs[i], ivec2(0), ivec2(hsize) - 1);
		float cd =
			viewDist((vec2(c) + 0.5) / hsize, texelFetch(minDepthHalf, c, 0).r);
		// Silhouette clamp, drop candidates whose surface is more than ~0.5 m
		// from this pixel's - that cross-edge pair is the halo source.
		float rw = abs(cd - fullDist) < 0.5 ? 1.0 : 0.0;
		float w = bw[i] * rw;
		cloudAccum += texelFetch(cloudRGBA, c, 0) * w;
		godrayAccum += texelFetch(godrayRGB, c, 0).rgb * w;
		godrayBilinear += texelFetch(godrayRGB, c, 0).rgb * bw[i];
		wsum += w;
	}

	vec4 cloud;
	vec3 godray;
	if (wsum > 0.0)
	{
		cloud = cloudAccum / wsum;
		godray = godrayAccum / wsum;
	}
	else
	{
		// At an edge none of the four samples match. Clouds just show nothing
		// here so they don't smear onto the foreground. Godrays instead blend
		// normally because rejecting them punched black holes in thin foliage, and a
		// soft shaft hides a halo better than it hides a hole.
		cloud = vec4(0.0, 0.0, 0.0, 1.0);
		godray = godrayBilinear;
	}

	vec3 hdr = lit * cloud.a + cloud.rgb + godray;

	// Fade opaque geometry toward sky color with distance. Sky pixels (depth ==
	// far) already hold real sky, so skip them to avoid double-applying.
	if (sceneDepth < 1.0)
	{
		vec3 wp = worldPos(vUv, sceneDepth);
		float distRamp = clamp((fullDist - apStart) / apFalloff, 0.0, 1.0);
		float heightFade = 1.0 - clamp(wp.y / apCanopyFade, 0.0, 1.0);
		float apWeight = apStrength * distRamp * heightFade;
		hdr = mix(hdr, sampleSky(normalize(wp - camPos)), apWeight);
	}

	outColor = vec4(hdr, 1.0);
}
