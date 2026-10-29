#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

inline float sampleHeightField(const std::vector<float>& field, int S,
							   float half, float worldSize, float wx, float wz)
{
	float u = (wx + half) / worldSize;
	float v = (wz + half) / worldSize;
	float fx = std::clamp(u, 0.0f, 1.0f) * (S - 1);
	float fz = std::clamp(v, 0.0f, 1.0f) * (S - 1);
	int x0 = (int)fx, z0 = (int)fz;
	int x1 = std::min(x0 + 1, S - 1), z1 = std::min(z0 + 1, S - 1);
	float tx = fx - x0, tz = fz - z0;
	float h00 = field[(size_t)z0 * S + x0];
	float h10 = field[(size_t)z0 * S + x1];
	float h01 = field[(size_t)z1 * S + x0];
	float h11 = field[(size_t)z1 * S + x1];
	float top = h00 + (h10 - h00) * tx;
	float bot = h01 + (h11 - h01) * tx;
	return top + (bot - top) * tz;
}
