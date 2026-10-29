#pragma once

class Vec4
{
public:
	float x, y, z, w;

	Vec4();
	Vec4(float x, float y, float z, float w);

	Vec4 operator+(const Vec4& rhs) const;
	Vec4 operator-(const Vec4& rhs) const;
	Vec4 operator*(float scalar) const;

	void operator+=(const Vec4& rhs);
	void operator*=(float scalar);

	Vec4 cross(const Vec4& rhs) const;
	Vec4 normalize() const;
	float dot(const Vec4& rhs) const;
};
