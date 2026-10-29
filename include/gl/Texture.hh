#pragma once
#include <glad/glad.h>

class Texture
{
private:
	GLuint _id;
	int _width;
	int _height;
	GLenum _internalFormat;

public:
	Texture(int width, int height, GLenum internalFormat,
			GLenum filtering = GL_NEAREST, GLenum wrap = GL_CLAMP_TO_EDGE,
			const char* label = nullptr);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&&) noexcept;
	Texture& operator=(Texture&&) noexcept;

	void bindToUnit(int unit) const;
	void bindAsImage(int unit, GLenum access) const;

	GLuint id() const
	{
		return _id;
	}
	int width() const
	{
		return _width;
	}
	int height() const
	{
		return _height;
	}
};
