#pragma once

#include "helpers/Matrix4.hh"

Matrix4 lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY,
			   float centerZ, float upX, float upY, float upZ);
