#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "helpers/Matrix4.hh"

class Camera
{
private:
	void processInput(GLFWwindow* window, float dt);
	void recomputeViewMatrix();

	float _yaw;
	float _pitch;
	float _position[3];

	float _lastMouseX;
	float _lastMouseY;
	bool _firstMouse;

	float _speed = 20.0f;

	Matrix4 _viewMatrix;

public:
	Camera(float startX, float startY, float startZ);

	void update(GLFWwindow* window, float dt);

	// Drop the stored cursor delta so re-grabbing the mouse doesn't snap the
	// view.
	void resetMouseState()
	{
		_firstMouse = true;
	}

	const float* getViewMatrix() const
	{
		return _viewMatrix.m;
	}
};
