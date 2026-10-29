#pragma once

// Single source of truth for visual tuning, defaults are the base-day preset.
struct StyleParams
{
	float sunColor[3] = { 1.0f, 0.95f, 0.88f };
	float sunIntensity = 3.5f;
	float turbidity = 2.5f;
	// HW emits near-physical luminance, so scale into HDR range.
	float skyRadianceScale = 0.05f;
	float skyAmbientColor[3] = { 0.24f, 0.285f, 0.36f };

	// Tuned to the 256 m terrain extent so distant canopies dissolve into sky.
	float aerialPerspectiveStrength = 0.35f;
	float aerialPerspectiveStart = 40.0f;
	float aerialPerspectiveFalloff = 120.0f;
	float aerialPerspectiveCanopyFade = 40.0f;

	float cloudAmbientColor[3] = { 0.55f, 0.58f, 0.68f };
	float cloudCoverage = 0.40f;
	float cloudDensityScale = 1.0f;
	float cloudDetailWeight = 0.40f;
	float cloudPhaseG = 0.70f;

	float godrayColor[3] = { 1.0f, 0.95f, 0.88f };
	float godrayIntensity = 4.0f;
	float godrayPhaseG =
		0.5f; // forward scatter, still readable off the sun axis
	float godrayMaxDistance = 60.0f;
	// weight by transmittance so in-scatter saturates rather than growing flat
	// with path length
	float godrayDensity = 0.05f;

	float grassTipColor[3] = { 0.55f, 0.72f, 0.32f };
	float grassBaseColor[3] = { 0.15f, 0.28f, 0.10f };
	float grassHueVariation = 0.12f;
	float grassDryness = 0.20f;
	float grassWindStrength = 1.0f;
	float grassWindGustiness = 0.6f;

	float shadowStrength = 0.80f;
	float shadowMaxDistance = 150.0f;
	float specularStrength = 0.05f;
	float foliageTranslucency = 1.2f;

	float bloomThreshold = 1.2f;
	float bloomStrength = 0.70f;
	float gradeLift[3] = { 0.01f, 0.01f, 0.015f };
	float gradeGamma[3] = { 1.0f, 1.0f, 1.02f };
	float gradeGain[3] = { 1.03f, 1.01f, 0.98f };
	float highlightTint[3] = { 1.02f, 1.0f, 0.98f };
	float shadowTint[3] = { 0.0f, 0.015f, 0.04f };
	float vignetteStrength = 0.25f;
};
