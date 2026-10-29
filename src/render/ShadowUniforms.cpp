#include "render/ShadowUniforms.hh"

#include <cstring>

#include "helpers/Matrix4.hh"

namespace
{
	constexpr GLsizeiptr kSize = sizeof(float) * 16;
} // namespace

ShadowUniforms::ShadowUniforms()
{
	glGenBuffers(1, &_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	glBufferData(GL_UNIFORM_BUFFER, kSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, kBinding, _ubo);

	const char* label = "ShadowUniforms UBO";
	glObjectLabel(GL_BUFFER, _ubo, (GLsizei)std::strlen(label), label);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

ShadowUniforms::~ShadowUniforms()
{
	glDeleteBuffers(1, &_ubo);
}

void ShadowUniforms::update(const Matrix4& lightSpaceMat)
{
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, kSize, lightSpaceMat.m);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
