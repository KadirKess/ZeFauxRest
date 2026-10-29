#pragma once

#include "render/StyleParams.hh"

struct RenderContext
{
	const float* viewMatrix = nullptr;
	const float* projMatrix = nullptr;

	float sunDir[3] = { 0.0f, 1.0f, 0.0f };

	float time = 0.0f;
	float windDirection[2] = { 1.0f, 0.0f };
	float windStrength = 1.0f;
	float windGustFrequency = 0.6f;

	StyleParams style;
};
