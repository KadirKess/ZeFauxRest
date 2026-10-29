#pragma once

#include "gl/Shader.hh"
#include "gl/Texture3D.hh"

// shape is 128^3 RGBA8 (Perlin in R, Worley in GBA), detail is 32^3 R8
class CloudNoise
{
private:
	Texture3D _shape;
	Texture3D _detail;

public:
	static constexpr int kShapeSize = 128;
	static constexpr int kDetailSize = 32;

	CloudNoise();

	CloudNoise(const CloudNoise&) = delete;
	CloudNoise& operator=(const CloudNoise&) = delete;

	const Texture3D& shape() const
	{
		return _shape;
	}
	const Texture3D& detail() const
	{
		return _detail;
	}
};
