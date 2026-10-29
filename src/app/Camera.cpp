#include "app/Camera.hh"

#include <cmath>

#include "helpers/LookAt.hh"
#include "helpers/Trigo.hh"

Camera::Camera(float startX, float startY, float startZ)
	: _yaw(0.0f)
	, _pitch(0.0f)
	, _position{ startX, startY, startZ }
	, _lastMouseX(0.0f)
	, _lastMouseY(0.0f)
	, _firstMouse(true)
{
	recomputeViewMatrix();
}

void Camera::update(GLFWwindow* window, float dt)
{
	processInput(window, dt);
	recomputeViewMatrix();
}

void Camera::processInput(GLFWwindow* window, float dt)
{
	float cosYaw = std::cos(radians(_yaw));
	float sinYaw = std::sin(radians(_yaw));
	float frontX = cosYaw;
	float frontZ = sinYaw;
	float rightX = -sinYaw;
	float rightZ = cosYaw;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		_position[0] += frontX * _speed * dt;
		_position[2] += frontZ * _speed * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		_position[0] -= frontX * _speed * dt;
		_position[2] -= frontZ * _speed * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		_position[0] -= rightX * _speed * dt;
		_position[2] -= rightZ * _speed * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		_position[0] += rightX * _speed * dt;
		_position[2] += rightZ * _speed * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		_position[1] += _speed * dt;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		_position[1] -= _speed * dt;

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (_firstMouse)
	{
		_lastMouseX = (float)mouseX;
		_lastMouseY = (float)mouseY;
		_firstMouse = false;
		return;
	}

	const float sensitivity = 0.1f;
	_yaw += ((float)mouseX - _lastMouseX) * sensitivity;
	_pitch += (_lastMouseY - (float)mouseY) * sensitivity;

	_lastMouseX = (float)mouseX;
	_lastMouseY = (float)mouseY;

	if (_pitch > 89.0f)
		_pitch = 89.0f;
	if (_pitch < -89.0f)
		_pitch = -89.0f;
}

void Camera::recomputeViewMatrix()
{
	float cosPitch = std::cos(radians(_pitch));
	float sinPitch = std::sin(radians(_pitch));
	float cosYaw = std::cos(radians(_yaw));
	float sinYaw = std::sin(radians(_yaw));

	float targetX = _position[0] + cosYaw * cosPitch;
	float targetY = _position[1] + sinPitch;
	float targetZ = _position[2] + sinYaw * cosPitch;

	_viewMatrix = lookAt(_position[0], _position[1], _position[2], targetX,
						 targetY, targetZ, 0.0f, 1.0f, 0.0f);
}
