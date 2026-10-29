#pragma once

#include "render/StyleParams.hh"

struct ColorGradePreset
{
	const char* name;
	float gradeLift[3];
	float gradeGamma[3];
	float gradeGain[3];
	float highlightTint[3];
	float shadowTint[3];
	float vignetteStrength;
};

inline const ColorGradePreset kColorGradePresets[] = {
	{ "Preset 1",
	  { 0.01f, 0.01f, 0.015f },
	  { 1.0f, 1.0f, 1.02f },
	  { 1.03f, 1.01f, 0.98f },
	  { 1.02f, 1.0f, 0.98f },
	  { 0.0f, 0.015f, 0.04f },
	  0.25f },
	{ "Preset 2",
	  { 0.02f, 0.01f, 0.0f },
	  { 1.0f, 1.0f, 0.98f },
	  { 1.08f, 1.0f, 0.92f },
	  { 1.08f, 1.0f, 0.9f },
	  { 0.03f, 0.012f, 0.0f },
	  0.30f },
	{ "Preset 3",
	  { 0.0f, 0.01f, 0.03f },
	  { 1.02f, 1.0f, 0.98f },
	  { 0.95f, 0.98f, 1.08f },
	  { 0.95f, 0.98f, 1.1f },
	  { 0.0f, 0.02f, 0.06f },
	  0.35f },
	{ "Preset 4",
	  { 0.0f, 0.005f, 0.01f },
	  { 1.0f, 1.0f, 1.0f },
	  { 1.05f, 1.0f, 0.95f },
	  { 1.1f, 1.02f, 0.9f },
	  { 0.0f, 0.03f, 0.05f },
	  0.30f },
	{ "Preset 5",
	  { 0.0f, 0.015f, 0.0f },
	  { 1.02f, 0.98f, 1.02f },
	  { 0.98f, 1.05f, 0.96f },
	  { 1.0f, 1.05f, 0.95f },
	  { 0.0f, 0.03f, 0.02f },
	  0.28f },
	{ "Preset 6",
	  { 0.03f, 0.02f, 0.0f },
	  { 0.98f, 1.0f, 1.05f },
	  { 1.1f, 1.02f, 0.88f },
	  { 1.12f, 1.04f, 0.85f },
	  { 0.04f, 0.02f, 0.0f },
	  0.32f },
	{ "Preset 7",
	  { -0.01f, -0.01f, 0.0f },
	  { 1.1f, 1.1f, 1.08f },
	  { 1.05f, 1.03f, 1.0f },
	  { 1.02f, 1.0f, 1.02f },
	  { 0.0f, 0.01f, 0.04f },
	  0.45f },
};

inline constexpr int kColorGradePresetCount =
	sizeof(kColorGradePresets) / sizeof(kColorGradePresets[0]);

inline void applyColorGradePreset(StyleParams& s, const ColorGradePreset& p)
{
	for (int i = 0; i < 3; ++i)
	{
		s.gradeLift[i] = p.gradeLift[i];
		s.gradeGamma[i] = p.gradeGamma[i];
		s.gradeGain[i] = p.gradeGain[i];
		s.highlightTint[i] = p.highlightTint[i];
		s.shadowTint[i] = p.shadowTint[i];
	}
	s.vignetteStrength = p.vignetteStrength;
}
