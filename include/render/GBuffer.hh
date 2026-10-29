#pragma once

#include "gl/Framebuffer.hh"
#include "gl/Texture.hh"

class GBuffer
{
private:
	Framebuffer _fbo;
	Texture _albedo;
	Texture _normal;
	Texture _depth;

public:
	GBuffer(int width, int height);

	GBuffer(const GBuffer&) = delete;
	GBuffer& operator=(const GBuffer&) = delete;

	void bind() const;
	void unbind() const;

	const Texture& albedo() const
	{
		return _albedo;
	}
	const Texture& normal() const
	{
		return _normal;
	}
	const Texture& depth() const
	{
		return _depth;
	}

	GLuint fboId() const
	{
		return _fbo.id();
	}
};
