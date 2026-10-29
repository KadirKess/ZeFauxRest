#include "scene/Grass.hh"

#include <algorithm>
#include <cstddef>
#include <glad/glad.h>
#include <random>
#include <vector>

#include "app/RenderContext.hh"
#include "gl/Texture.hh"
#include "helpers/Matrix4.hh"
#include "helpers/TerrainSample.hh"
#include "scene/Terrain.hh"

// A blade is two crossed quads, both are drawn so it reads volumetric from any
// horizontal angle instead of vanishing edge-on.
static constexpr GLsizei kBladeQuadIndices = 12;

Mesh Grass::buildBlade()
{
	const float w = 0.05f;
	const float h = 0.6f;
	const float mid = 0.5f * h;

	std::vector<float> vertices = {
		-w,	  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, w,	  0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 0.0f, -w,	  mid,	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f,
		w,	  mid,	0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, -w,	  h,	0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, w,	  h,	0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

		0.0f, 0.0f, -w,	  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, w,	  1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, mid,	-w,	  1.0f, 0.0f, 0.0f, 0.0f, 0.5f,
		0.0f, mid,	w,	  1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, h,	-w,	  1.0f,
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, h,	w,	  1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	};

	std::vector<unsigned int> indices = {
		0, 1, 2, 2, 1, 3, 2, 3, 4,	4,	3, 5,
		6, 7, 8, 8, 7, 9, 8, 9, 10, 10, 9, 11,
	};

	return Mesh(vertices, indices);
}

Grass::Grass(const Terrain& terrain)
	: _mesh(buildBlade())
	, _shader(SHADER_DIR "/passes/gbuffer_grass.vert",
			  SHADER_DIR "/passes/gbuffer_grass.frag")
	, _instanceVbo(0)
	, _instanceCount(0)
	, _drawIndexCount(2 * kBladeQuadIndices)
{
	scatter(terrain);
}

Grass::~Grass()
{
	glDeleteBuffers(1, &_instanceVbo);
}

void Grass::scatter(const Terrain& terrain)
{
	const int S = Terrain::kHeightmapSize;
	const float worldSize = Terrain::kWorldSize;
	const float half = worldSize * 0.5f;
	const float heightScale = Terrain::kHeightScale;

	// Read the GPU heightmap back so blades sit on the surface (Terrain built
	// it in its constructor).
	std::vector<float> heights((size_t)S * S);
	glGetTextureImage(terrain.heightmap().id(), 0, GL_RED, GL_FLOAT,
					  (GLsizei)(heights.size() * sizeof(float)),
					  heights.data());

	// Full-density carpet, distance falloff is a GPU per-blade vertex-kill
	// against blue noise, so the CPU list stays dense.
	const int target = 1000000;
	const int maxAttempts = target * 2;

	std::mt19937 rng(1337u);
	std::uniform_real_distribution<float> posDist(-half, half);
	std::uniform_real_distribution<float> yawDist(0.0f, 6.2831853f);
	std::uniform_real_distribution<float> heightDist(0.7f, 1.1f);
	std::uniform_real_distribution<float> seedDist(0.0f, 1.0f);

	std::vector<GrassInstance> instances;
	instances.reserve(target);
	for (int a = 0; a < maxAttempts && (int)instances.size() < target; ++a)
	{
		float x = posDist(rng);
		float z = posDist(rng);

		GrassInstance inst{};
		inst.pos[0] = x;
		inst.pos[1] =
			sampleHeightField(heights, S, half, worldSize, x, z) * heightScale;
		inst.pos[2] = z;
		inst.yaw = yawDist(rng);
		inst.heightScale = heightDist(rng);
		inst.colorSeed = seedDist(rng);
		instances.push_back(inst);
	}
	_instanceCount = (GLsizei)instances.size();

	glGenBuffers(1, &_instanceVbo);
	glBindVertexArray(_mesh.vao());
	glBindBuffer(GL_ARRAY_BUFFER, _instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(GrassInstance),
				 instances.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GrassInstance),
						  (void*)offsetof(GrassInstance, pos));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GrassInstance),
						  (void*)offsetof(GrassInstance, yaw));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);

	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(GrassInstance),
						  (void*)offsetof(GrassInstance, heightScale));
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);

	glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(GrassInstance),
						  (void*)offsetof(GrassInstance, colorSeed));
	glEnableVertexAttribArray(6);
	glVertexAttribDivisor(6, 1);

	glObjectLabel(GL_BUFFER, _instanceVbo, -1, "Grass instance VBO");
	glObjectLabel(GL_VERTEX_ARRAY, _mesh.vao(), -1, "Grass VAO");

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Grass::draw(const float* mvp, const RenderContext& ctx,
				 const Texture& blueNoise) const
{
	const StyleParams& style = ctx.style;

	_shader.use();
	_shader.setUniformMat4("modelViewProjection", mvp);

	Matrix4 view;
	for (int i = 0; i < 16; ++i)
		view.m[i] = ctx.viewMatrix[i];
	Matrix4 invView = inverse(view);
	float cameraPos[3] = { invView.m[3], invView.m[7], invView.m[11] };
	_shader.setUniformVec3("cameraPos", cameraPos);

	_shader.setUniformVec3("grassBaseColor", style.grassBaseColor);
	_shader.setUniformVec3("grassTipColor", style.grassTipColor);
	_shader.setUniform1f("grassHueVariation", style.grassHueVariation);
	_shader.setUniform1f("grassDryness", style.grassDryness);

	blueNoise.bindToUnit(0);
	_shader.setUniform1i("uBlueNoise", 0);

	glBindVertexArray(_mesh.vao());
	glDrawElementsInstanced(GL_TRIANGLES, _drawIndexCount, GL_UNSIGNED_INT,
							nullptr, _instanceCount);
	glBindVertexArray(0);
}
