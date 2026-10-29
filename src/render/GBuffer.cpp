#include "render/GBuffer.hh"

#include <stdexcept>

GBuffer::GBuffer(int width, int height)
	: _fbo("GBuffer FBO")
	, _albedo(width, height, GL_RGBA8, GL_NEAREST, GL_CLAMP_TO_EDGE, "gAlbedo")
	, _normal(width, height, GL_RGBA16F, GL_NEAREST, GL_CLAMP_TO_EDGE,
			  "gNormal")
	, _depth(width, height, GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_CLAMP_TO_EDGE,
			 "gDepth")
{
	_fbo.bind();
	_fbo.attachColor(_albedo, 0);
	_fbo.attachColor(_normal, 1);
	_fbo.attachDepth(_depth);
	_fbo.setDrawBuffers(2);

	if (!_fbo.isComplete())
		throw std::runtime_error("GBuffer FBO is incomplete");

	_fbo.unbind();
}

void GBuffer::bind() const
{
	_fbo.bind();
}

void GBuffer::unbind() const
{
	_fbo.unbind();
}
