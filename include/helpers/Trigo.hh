#pragma once

#include <cmath>

#ifndef M_PIf
#	define M_PIf 3.14159265358979323846f
#endif

inline float radians(float degrees)
{
	return degrees * (M_PIf / 180.0f);
}
