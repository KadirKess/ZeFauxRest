#include "helpers/Ortho.hh"

Matrix4 ortho(float left, float right, float bottom, float top, float znear,
			  float zfar)
{
	Matrix4 result;

	result[0] = 2.0f / (right - left);
	result[3] = -(right + left) / (right - left);
	result[5] = 2.0f / (top - bottom);
	result[7] = -(top + bottom) / (top - bottom);
	result[10] = -2.0f / (zfar - znear);
	result[11] = -(zfar + znear) / (zfar - znear);
	result[15] = 1.0f;

	return result;
}
