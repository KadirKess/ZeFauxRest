#version 450

#include "../shared/wind.glsl"

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

// Per-instance (divisor 1).
layout(location = 3) in vec3 iPos;
layout(location = 4) in float iYaw;
layout(location = 5) in float iHeightScale;
layout(location = 6) in float iColorSeed;

uniform mat4 modelViewProjection;
uniform vec3 cameraPos;
uniform sampler2D uBlueNoise;

out vec3 fragNormal;
out float fragHeight;
out float fragSeed;

const float TAU = 6.2831853;

void main()
{
	// One noise value per blade, read at its position so every vertex of the
	// blade agrees on its sway and whether it's kept. Scaled so neighbouring
	// blades land on neighbouring texels of the tiled 64x64 noise.
	float falloffNoise = texture(uBlueNoise, iPos.xz * 4.0).r;
	float phaseNoise = texture(uBlueNoise, iPos.xz * 4.0 + vec2(37.0, 17.0)).r;

	// Thin the grass out with distance so more blades drop as the ramp rises from
	// near to far. Using each blade's own noise to decide makes the edge
	// dissolve gradually instead of forming a hard line.
	float dist = distance(cameraPos, iPos);
	if (falloffNoise < smoothstep(80.0, 150.0, dist))
	{
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // off-screen, clipped
		return;
	}

	float t = vUV.y;

	vec3 local = vPosition;
	local.y *= iHeightScale;

	float c = cos(iYaw);
	float s = sin(iYaw);
	vec3 p =
		vec3(c * local.x + s * local.z, local.y, -s * local.x + c * local.z);
	vec3 worldPos = iPos + p;

	// Phase decorrelation, roll each blade on its own wind phase so 1M blades
	// don't sweep in synchronized bands inside a gust.
	WindParams wp = windParams();
	wp.time += phaseNoise * TAU;

	// Quadratic weight anchors the base, throws the tip.
	worldPos += applyWind(worldPos, wp) * (t * t);

	gl_Position = modelViewProjection * vec4(worldPos, 1.0);

	fragNormal = vec3(c * vNormal.x + s * vNormal.z, vNormal.y,
					  -s * vNormal.x + c * vNormal.z);
	fragHeight = t;
	fragSeed = iColorSeed;
}
