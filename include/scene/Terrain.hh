#pragma once

#include "gl/Mesh.hh"
#include "gl/Shader.hh"
#include "gl/Texture.hh"

class Terrain
{
private:
	Mesh _mesh;
	Texture _heightmap;
	Texture _coverageMask;
	Shader _shader;
	Shader _heightmapCompute;
	Shader _coverageCompute;

	static Mesh buildGrid(int patches, float worldSize);

	void bindHeightField(const Shader& shader) const;

public:
	static constexpr int kHeightmapSize = 1024;
	static constexpr int kPatches = 256;
	static constexpr float kWorldSize = 256.0f;
	static constexpr float kHeightScale = 15.0f;

	static constexpr int kHeightmapUnit = 0;
	static constexpr int kCoverageMaskUnit = 1;

	Terrain();

	Terrain(const Terrain&) = delete;
	Terrain& operator=(const Terrain&) = delete;

	void draw(const float* mvp) const;
	void drawDepthOnly(const Shader& shader) const;

	const Texture& heightmap() const
	{
		return _heightmap;
	}
	const Texture& coverageMask() const
	{
		return _coverageMask;
	}
};
