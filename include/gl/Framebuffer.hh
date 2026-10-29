#pragma once

#include <glad/glad.h>

#include "gl/Texture.hh"

class Framebuffer
{
private:
	GLuint _id;

public:
	Framebuffer(const char* label = nullptr);
	~Framebuffer();

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

	Framebuffer(Framebuffer&&) noexcept;
	Framebuffer& operator=(Framebuffer&&) noexcept;

	void attachColor(const Texture& texture, int slot = 0) const;
	void attachDepth(const Texture& texture) const;
	void setDrawBuffers(int numBuffers) const;

	void bind() const;
	void unbind() const;
	bool isComplete() const;

	GLuint id() const
	{
		return _id;
	}
};
