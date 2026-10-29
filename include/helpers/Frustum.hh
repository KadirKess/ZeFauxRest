#pragma once

#include "helpers/Matrix4.hh"

Matrix4 frustum(float left, float right, float bottom, float top, float znear,
				float zfar);
