#include "helpers/LookAt.hh"

#include "helpers/Vec4.hh"

Matrix4 lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY,
			   float centerZ, float upX, float upY, float upZ)
{
	Vec4 eye{ eyeX, eyeY, eyeZ, 0.0f };
	Vec4 center{ centerX, centerY, centerZ, 0.0f };
	Vec4 up{ upX, upY, upZ, 0.0f };

	Vec4 f = (center - eye).normalize();
	Vec4 s = f.cross(up).normalize();
	Vec4 u = s.cross(f).normalize();

	Matrix4 rotation;
	rotation[0] = s.x;
	rotation[1] = s.y;
	rotation[2] = s.z;
	rotation[3] = 0.0f;
	rotation[4] = u.x;
	rotation[5] = u.y;
	rotation[6] = u.z;
	rotation[7] = 0.0f;
	rotation[8] = -f.x;
	rotation[9] = -f.y;
	rotation[10] = -f.z;
	rotation[11] = 0.0f;
	rotation[12] = 0.0f;
	rotation[13] = 0.0f;
	rotation[14] = 0.0f;
	rotation[15] = 1.0f;

	Matrix4 translation;
	translation[0] = 1.0f;
	translation[1] = 0.0f;
	translation[2] = 0.0f;
	translation[3] = -eye.x;
	translation[4] = 0.0f;
	translation[5] = 1.0f;
	translation[6] = 0.0f;
	translation[7] = -eye.y;
	translation[8] = 0.0f;
	translation[9] = 0.0f;
	translation[10] = 1.0f;
	translation[11] = -eye.z;
	translation[12] = 0.0f;
	translation[13] = 0.0f;
	translation[14] = 0.0f;
	translation[15] = 1.0f;

	return rotation * translation;
}
