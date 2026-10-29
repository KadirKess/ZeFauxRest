#include "render/WindUniforms.hh"

#include <cstring>

#include "app/RenderContext.hh"

namespace
{
	struct WindUboLayout
	{
		float time;
		float _pad0;
		float direction[2];
		float strength;
		float gustFrequency;
		float _pad1[2];
	};
} // namespace

WindUniforms::WindUniforms()
{
	glGenBuffers(1, &_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(WindUboLayout), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, kBinding, _ubo);

	const char* label = "WindUniforms UBO";
	glObjectLabel(GL_BUFFER, _ubo, (GLsizei)std::strlen(label), label);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

WindUniforms::~WindUniforms()
{
	glDeleteBuffers(1, &_ubo);
}

void WindUniforms::update(const RenderContext& ctx)
{
	WindUboLayout data{};
	data.time = ctx.time;
	data.direction[0] = ctx.windDirection[0];
	data.direction[1] = ctx.windDirection[1];
	data.strength = ctx.windStrength;
	data.gustFrequency = ctx.windGustFrequency;

	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(WindUboLayout), &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
