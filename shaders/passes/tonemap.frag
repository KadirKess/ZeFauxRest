#version 450

in vec2 vUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D hdrComposite;
uniform sampler2D bloom;
uniform float bloomStrength;

// Hable not ACES - ACES rotates saturated greens toward yellow and crunches the
// grass color
vec3 hable(vec3 x)
{
	const float A = 0.15, B = 0.50, C = 0.10, D = 0.20, E = 0.02, F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main()
{
	vec3 hdr = texture(hdrComposite, vUv).rgb;

	// bloom before tonemap so it participates in the curve
	hdr += bloomStrength * texture(bloom, vUv).rgb;

	const float W = 11.2; // linear white point
	vec3 mapped = hable(hdr) / hable(vec3(W));

	outColor = vec4(mapped, 1.0);
}
