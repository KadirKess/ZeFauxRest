#include "helpers/Vec4.hh"

#include <cmath>

Vec4::Vec4()
	: x(0.0f)
	, y(0.0f)
	, z(0.0f)
	, w(0.0f)
{}

Vec4::Vec4(float x, float y, float z, float w)
	: x(x)
	, y(y)
	, z(z)
	, w(w)
{}

Vec4 Vec4::operator+(const Vec4& rhs) const
{
	return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

Vec4 Vec4::operator-(const Vec4& rhs) const
{
	return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

Vec4 Vec4::operator*(float scalar) const
{
	return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

void Vec4::operator+=(const Vec4& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
}

void Vec4::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
}

Vec4 Vec4::cross(const Vec4& rhs) const
{
	return Vec4(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
				x * rhs.y - y * rhs.x, 0.0f);
}

Vec4 Vec4::normalize() const
{
	float length = std::sqrt(x * x + y * y + z * z);
	return Vec4(x / length, y / length, z / length, w);
}

float Vec4::dot(const Vec4& rhs) const
{
	return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
}
