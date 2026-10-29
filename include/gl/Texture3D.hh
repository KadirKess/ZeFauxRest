#pragma once
#include <glad/glad.h>

// Binds the whole volume as a layered image so a compute shader can write it.
class Texture3D
{
private:
	GLuint _id;
	int _width;
	int _height;
	int _depth;
	GLenum _internalFormat;

public:
	Texture3D(int width, int height, int depth, GLenum internalFormat,
			  GLenum filtering = GL_LINEAR, GLenum wrap = GL_REPEAT,
			  const char* label = nullptr);
	~Texture3D();

	Texture3D(const Texture3D&) = delete;
	Texture3D& operator=(const Texture3D&) = delete;

	Texture3D(Texture3D&&) noexcept;
	Texture3D& operator=(Texture3D&&) noexcept;

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
	int depth() const
	{
		return _depth;
	}
};
