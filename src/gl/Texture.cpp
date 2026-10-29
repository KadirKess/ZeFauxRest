#include "gl/Texture.hh"

#include <cstring>

Texture::Texture(int width, int height, GLenum internalFormat, GLenum filtering,
				 GLenum wrap, const char* label)
	: _width(width)
	, _height(height)
	, _internalFormat(internalFormat)
{
	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	if (label)
		glObjectLabel(GL_TEXTURE, _id, (GLsizei)std::strlen(label), label);
}

Texture::~Texture()
{
	glDeleteTextures(1, &_id);
}

Texture::Texture(Texture&& other) noexcept
	: _id(other._id)
	, _width(other._width)
	, _height(other._height)
	, _internalFormat(other._internalFormat)
{
	other._id = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this != &other)
	{
		glDeleteTextures(1, &_id);

		_id = other._id;
		_width = other._width;
		_height = other._height;
		_internalFormat = other._internalFormat;

		other._id = 0;
	}
	return *this;
}

void Texture::bindToUnit(int unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, _id);
}

void Texture::bindAsImage(int unit, GLenum access) const
{
	glBindImageTexture(unit, _id, 0, GL_FALSE, 0, access, _internalFormat);
}
