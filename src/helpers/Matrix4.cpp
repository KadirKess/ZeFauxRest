#include "helpers/Matrix4.hh"

#include <cmath>
#include <utility>

#include "helpers/Vec4.hh"

Matrix4::Matrix4()
{
	for (int i = 0; i < 16; ++i)
		m[i] = 0.0f;
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
	Matrix4 result;
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			for (int k = 0; k < 4; ++k)
				result.m[row * 4 + col] += m[row * 4 + k] * rhs.m[k * 4 + col];
	return result;
}

Matrix4 inverse(const Matrix4& src)
{
	double a[4][8];
	for (int r = 0; r < 4; ++r)
		for (int c = 0; c < 4; ++c)
		{
			a[r][c] = src.m[r * 4 + c];
			a[r][4 + c] = (r == c) ? 1.0 : 0.0;
		}

	for (int col = 0; col < 4; ++col)
	{
		int pivot = col;
		for (int r = col + 1; r < 4; ++r)
			if (std::fabs(a[r][col]) > std::fabs(a[pivot][col]))
				pivot = r;
		if (pivot != col)
			for (int c = 0; c < 8; ++c)
				std::swap(a[col][c], a[pivot][c]);

		double d = a[col][col];
		for (int c = 0; c < 8; ++c)
			a[col][c] /= d;

		for (int r = 0; r < 4; ++r)
		{
			if (r == col)
				continue;
			double f = a[r][col];
			for (int c = 0; c < 8; ++c)
				a[r][c] -= f * a[col][c];
		}
	}

	Matrix4 inv;
	for (int r = 0; r < 4; ++r)
		for (int c = 0; c < 4; ++c)
			inv.m[r * 4 + c] = (float)a[r][4 + c];
	return inv;
}

Vec4 Matrix4::operator*(const Vec4& rhs) const
{
	return Vec4(m[0] * rhs.x + m[1] * rhs.y + m[2] * rhs.z + m[3] * rhs.w,
				m[4] * rhs.x + m[5] * rhs.y + m[6] * rhs.z + m[7] * rhs.w,
				m[8] * rhs.x + m[9] * rhs.y + m[10] * rhs.z + m[11] * rhs.w,
				m[12] * rhs.x + m[13] * rhs.y + m[14] * rhs.z + m[15] * rhs.w);
}
