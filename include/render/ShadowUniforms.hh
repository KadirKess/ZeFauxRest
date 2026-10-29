#pragma once

#include <glad/glad.h>

class Matrix4;

// single source of truth for the shadow matrix, deferred lighting and godrays
// both read it
class ShadowUniforms
{
private:
	GLuint _ubo = 0;

public:
	static constexpr GLuint kBinding = 1; // distinct from Wind at binding=0

	ShadowUniforms();
	~ShadowUniforms();

	ShadowUniforms(const ShadowUniforms&) = delete;
	ShadowUniforms& operator=(const ShadowUniforms&) = delete;

	void update(const Matrix4& lightSpaceMat);
};
