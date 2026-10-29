#include "render/SkyModel.hh"

#include <algorithm>
#include <cmath>

#include "ArHosekSkyModelData_RGB.h"
#include "gl/Shader.hh"

// Hosek-Wilkie stores coefficients as Bernstein control points - use quintic
// basis, not lerp
static double bezier5(const double* p, int stride, double t)
{
	double u = 1.0 - t;
	double u2 = u * u, u3 = u2 * u, u4 = u3 * u, u5 = u4 * u;
	double t2 = t * t, t3 = t2 * t, t4 = t3 * t, t5 = t4 * t;
	return u5 * p[0 * stride] + 5.0 * u4 * t * p[1 * stride]
		+ 10.0 * u3 * t2 * p[2 * stride] + 10.0 * u2 * t3 * p[3 * stride]
		+ 5.0 * u * t4 * p[4 * stride] + t5 * p[5 * stride];
}

void SkyModel::compute(float turbidity, float sunElevRad, float radScale)
{
	// Albedo is fixed at 0, so only the low-albedo half of each table is read.
	double turb = std::fmax(1.0, std::fmin((double)turbidity, 10.0));
	int intTurb = (int)turb;
	double turbRem = turb - (double)intTurb;
	int lowBlock = intTurb - 1; // turbidity 1 -> block 0
	bool hasHigh = intTurb < 10;
	int hiBlock = hasHigh ? intTurb : lowBlock;

	// Cube-root warp on solar elevation [0, pi/2] matches the reference cook.
	double elev = std::fmax(0.0, std::fmin((double)sunElevRad, M_PI_2));
	double se = std::pow(elev / M_PI_2, 1.0 / 3.0);

	const double* tables[3] = { datasetRGB1, datasetRGB2, datasetRGB3 };
	const double* radTables[3] = { datasetRGBRad1, datasetRGBRad2,
								   datasetRGBRad3 };
	float* coeffs[3] = { coeffsR, coeffsG, coeffsB };

	for (int ch = 0; ch < 3; ++ch)
	{
		const double* lowMat = tables[ch] + 9 * 6 * lowBlock;
		const double* hiMat = tables[ch] + 9 * 6 * hiBlock;
		for (int p = 0; p < 9; ++p)
		{
			double lo = bezier5(lowMat + p, 9, se);
			double hi = bezier5(hiMat + p, 9, se);
			coeffs[ch][p] = (float)((1.0 - turbRem) * lo + turbRem * hi);
		}

		double rLo = bezier5(radTables[ch] + 6 * lowBlock, 1, se);
		double rHi = bezier5(radTables[ch] + 6 * hiBlock, 1, se);
		rad[ch] = (float)((1.0 - turbRem) * rLo + turbRem * rHi) * radScale;
	}
}

void SkyModel::upload(const Shader& shader) const
{
	shader.setUniformVec3("hwR0", coeffsR + 0);
	shader.setUniformVec3("hwR1", coeffsR + 3);
	shader.setUniformVec3("hwR2", coeffsR + 6);
	shader.setUniformVec3("hwG0", coeffsG + 0);
	shader.setUniformVec3("hwG1", coeffsG + 3);
	shader.setUniformVec3("hwG2", coeffsG + 6);
	shader.setUniformVec3("hwB0", coeffsB + 0);
	shader.setUniformVec3("hwB1", coeffsB + 3);
	shader.setUniformVec3("hwB2", coeffsB + 6);
	shader.setUniformVec3("hwRad", rad);
}
