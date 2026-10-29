#include "gl/Framebuffer.hh"

#include <cstring>

Framebuffer::Framebuffer(const char* label)
{
	glGenFramebuffers(1, &_id);

	// A framebuffer name has no type until first bound, so labelling it before
	// the first bind is undefined on some drivers, bind once up front.
	if (label)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _id);
		glObjectLabel(GL_FRAMEBUFFER, _id, (GLsizei)std::strlen(label), label);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &_id);
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
	: _id(other._id)
{
	other._id = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
	if (this != &other)
	{
		glDeleteFramebuffers(1, &_id);
		_id = other._id;
		other._id = 0;
	}
	return *this;
}

void Framebuffer::attachColor(const Texture& texture, int slot) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot,
						   GL_TEXTURE_2D, texture.id(), 0);
}

void Framebuffer::attachDepth(const Texture& texture) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
						   texture.id(), 0);
}

void Framebuffer::setDrawBuffers(int numBuffers) const
{
	if (numBuffers == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		return;
	}

	GLenum buffers[8];
	for (int i = 0; i < numBuffers; ++i)
		buffers[i] = GL_COLOR_ATTACHMENT0 + i;
	glDrawBuffers(numBuffers, buffers);
}

void Framebuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void Framebuffer::unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::isComplete() const
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
