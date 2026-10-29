#pragma once

class Shader;

// Hosek-Wilkie coefficients shared across sky pass and aerial perspective so no
// horizon seam
struct SkyModel
{
	float coeffsR[9];
	float coeffsG[9];
	float coeffsB[9];
	float rad[3];

	void compute(float turbidity, float sunElevRad, float radScale);

	// Callers set sunDir themselves.
	void upload(const Shader& shader) const;
};
