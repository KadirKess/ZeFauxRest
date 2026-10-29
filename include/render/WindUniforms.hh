#pragma once

#include <glad/glad.h>

struct RenderContext;

class WindUniforms
{
private:
	GLuint _ubo = 0;

public:
	static constexpr GLuint kBinding = 0;

	WindUniforms();
	~WindUniforms();

	WindUniforms(const WindUniforms&) = delete;
	WindUniforms& operator=(const WindUniforms&) = delete;

	void update(const RenderContext& ctx);
};
