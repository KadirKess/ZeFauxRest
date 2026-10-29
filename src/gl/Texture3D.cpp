#include "gl/Texture3D.hh"

#include <cstring>

Texture3D::Texture3D(int width, int height, int depth, GLenum internalFormat,
					 GLenum filtering, GLenum wrap, const char* label)
	: _width(width)
	, _height(height)
	, _depth(depth)
	, _internalFormat(internalFormat)
{
	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_3D, _id);
	glTexStorage3D(GL_TEXTURE_3D, 1, internalFormat, width, height, depth);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap);

	if (label)
		glObjectLabel(GL_TEXTURE, _id, (GLsizei)std::strlen(label), label);
}

Texture3D::~Texture3D()
{
	glDeleteTextures(1, &_id);
}

Texture3D::Texture3D(Texture3D&& other) noexcept
	: _id(other._id)
	, _width(other._width)
	, _height(other._height)
	, _depth(other._depth)
	, _internalFormat(other._internalFormat)
{
	other._id = 0;
}

Texture3D& Texture3D::operator=(Texture3D&& other) noexcept
{
	if (this != &other)
	{
		glDeleteTextures(1, &_id);

		_id = other._id;
		_width = other._width;
		_height = other._height;
		_depth = other._depth;
		_internalFormat = other._internalFormat;

		other._id = 0;
	}
	return *this;
}

void Texture3D::bindToUnit(int unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_3D, _id);
}

void Texture3D::bindAsImage(int unit, GLenum access) const
{
	// layered = GL_TRUE binds the whole volume so the compute shader can write
	// every slice in one dispatch.
	glBindImageTexture(unit, _id, 0, GL_TRUE, 0, access, _internalFormat);
}
