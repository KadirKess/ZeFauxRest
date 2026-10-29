#include "scene/Terrain.hh"

#include <glad/glad.h>
#include <vector>

Mesh Terrain::buildGrid(int patches, float worldSize)
{
	int verts = patches + 1;
	float step = worldSize / (float)patches;
	float half = worldSize * 0.5f;

	std::vector<float> vertices;
	vertices.reserve((size_t)verts * verts * 8);

	for (int z = 0; z < verts; ++z)
	{
		for (int x = 0; x < verts; ++x)
		{
			float wx = -half + x * step;
			float wz = -half + z * step;
			float u = (float)x / (float)patches;
			float v = (float)z / (float)patches;
			// pos, normal (unused - frag shader derives from heightmap), uv
			vertices.insert(vertices.end(),
							{ wx, 0.0f, wz, 0.0f, 1.0f, 0.0f, u, v });
		}
	}

	std::vector<unsigned int> indices;
	indices.reserve((size_t)patches * patches * 6);

	for (int z = 0; z < patches; ++z)
	{
		for (int x = 0; x < patches; ++x)
		{
			unsigned int tl = z * verts + x;
			unsigned int tr = tl + 1;
			unsigned int bl = tl + verts;
			unsigned int br = bl + 1;
			indices.insert(indices.end(), { tl, bl, tr, tr, bl, br });
		}
	}

	return Mesh(vertices, indices);
}

Terrain::Terrain()
	: _mesh(buildGrid(kPatches, kWorldSize))
	, _heightmap(kHeightmapSize, kHeightmapSize, GL_R16F, GL_LINEAR,
				 GL_CLAMP_TO_EDGE, "heightmap")
	, _coverageMask(kHeightmapSize, kHeightmapSize, GL_R8, GL_LINEAR,
					GL_CLAMP_TO_EDGE, "coverageMask")
	, _shader(SHADER_DIR "/passes/gbuffer_terrain.vert",
			  SHADER_DIR "/passes/gbuffer_terrain.frag")
	, _heightmapCompute(
		  SHADER_DIR "/passes/heightmap.comp",
		  std::vector<std::string>{ "NOISE_MODE NOISE_MODE_FULL" })
	, _coverageCompute(SHADER_DIR "/passes/coverage.comp")
{
	_heightmap.bindAsImage(0, GL_WRITE_ONLY);
	_heightmapCompute.use();
	glDispatchCompute(kHeightmapSize / 16, kHeightmapSize / 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT);

	_heightmap.bindAsImage(0, GL_READ_ONLY);
	_coverageMask.bindAsImage(1, GL_WRITE_ONLY);
	_coverageCompute.use();
	_coverageCompute.setUniform1f("heightScale", kHeightScale);
	_coverageCompute.setUniform1f("worldStep",
								  kWorldSize / (float)kHeightmapSize);
	glDispatchCompute(kHeightmapSize / 16, kHeightmapSize / 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Terrain::bindHeightField(const Shader& shader) const
{
	shader.setUniform1f("heightScale", kHeightScale);
	shader.setUniform1f("texelSize", 1.0f / (float)kHeightmapSize);
	shader.setUniform1f("worldStep", kWorldSize / (float)kHeightmapSize);

	_heightmap.bindToUnit(kHeightmapUnit);
	shader.setUniform1i("heightMap", kHeightmapUnit);
}

void Terrain::draw(const float* mvp) const
{
	_shader.use();
	_shader.setUniformMat4("modelViewProjection", mvp);
	bindHeightField(_shader);

	float color[3] = { 0.35f, 0.28f, 0.18f };
	_shader.setUniformVec3("terrainColor", color);

	// Read here for ground-AO darkening under dense grass and left bound so
	// downstream placement samples a stable unit.
	_coverageMask.bindToUnit(kCoverageMaskUnit);
	_shader.setUniform1i("coverageMask", kCoverageMaskUnit);

	_mesh.draw();
}

void Terrain::drawDepthOnly(const Shader& shader) const
{
	shader.use();
	bindHeightField(shader);

	_mesh.draw();
}
