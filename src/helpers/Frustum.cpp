#include "helpers/Frustum.hh"

Matrix4 frustum(float left, float right, float bottom, float top, float znear,
				float zfar)
{
	Matrix4 result;

	result[0] = (2.0f * znear) / (right - left);
	result[2] = (right + left) / (right - left);
	result[5] = (2.0f * znear) / (top - bottom);
	result[6] = (top + bottom) / (top - bottom);
	result[10] = -(zfar + znear) / (zfar - znear);
	result[11] = -(2.0f * zfar * znear) / (zfar - znear);
	result[14] = -1.0f;

	return result;
}
