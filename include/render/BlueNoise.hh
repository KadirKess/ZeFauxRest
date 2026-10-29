#pragma once

#include "gl/Shader.hh"
#include "gl/Texture.hh"

// R8 tileable blue noise, shared by clouds and godrays
class BlueNoise
{
private:
	Shader _shader;
	Texture _texture;

public:
	static constexpr int kSize = 64;

	BlueNoise();

	BlueNoise(const BlueNoise&) = delete;
	BlueNoise& operator=(const BlueNoise&) = delete;

	const Texture& texture() const
	{
		return _texture;
	}
};
