#pragma once

#include "helpers/Matrix4.hh"

Matrix4 ortho(float left, float right, float bottom, float top, float znear,
			  float zfar);
