#pragma once

class Vec4;

class Matrix4
{
public:
	float m[16];

	Matrix4();

	float& operator[](int index)
	{
		return m[index];
	}
	const float& operator[](int index) const
	{
		return m[index];
	}

	Matrix4 operator*(const Matrix4& rhs) const;
	Vec4 operator*(const Vec4& rhs) const;
};

// raymarch passes need this to turn screen NDC back into world rays
Matrix4 inverse(const Matrix4& src);
