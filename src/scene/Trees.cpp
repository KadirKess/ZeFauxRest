#include "scene/Trees.hh"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <meshoptimizer.h>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "helpers/TerrainSample.hh"
#include "scene/ObjLoader.hh"
#include "scene/Terrain.hh"

static const char* const kModelFiles[] = {
	ASSET_DIR "/trees/mixofheightandleaves.obj",
};
static constexpr int kModelCount =
	(int)(sizeof(kModelFiles) / sizeof(kModelFiles[0]));

Trees::Trees(const Terrain& terrain)
	: _instanceVbo(0)
	, _shadowInstanceVbo(0)
	, _instanceCount(0)
	, _shader(SHADER_DIR "/passes/gbuffer_tree.vert",
			  SHADER_DIR "/passes/gbuffer_tree.frag")
{
	buildModels();
	scatter(terrain);

	for (size_t m = 0; m < _models.size(); ++m)
	{
		TreeModel& model = _models[m];
		setupVao(model.vao, model.vbo, model.ebo, _instanceVbo);
		setupVao(model.shadowVao, model.vbo, model.ebo, _shadowInstanceVbo);
		glObjectLabel(GL_VERTEX_ARRAY, model.vao, -1, "Trees VAO");
		glObjectLabel(GL_VERTEX_ARRAY, model.shadowVao, -1, "Trees shadow VAO");
		glObjectLabel(GL_BUFFER, model.vbo, -1, "Trees VBO");
		glObjectLabel(GL_BUFFER, model.ebo, -1, "Trees LOD EBO");
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Trees::~Trees()
{
	glDeleteBuffers(1, &_shadowInstanceVbo);
	glDeleteBuffers(1, &_instanceVbo);
	for (TreeModel& model : _models)
	{
		glDeleteBuffers(1, &model.ebo);
		glDeleteBuffers(1, &model.vbo);
		glDeleteVertexArrays(1, &model.shadowVao);
		glDeleteVertexArrays(1, &model.vao);
	}
}

void Trees::buildModels()
{
	_models.reserve(kModelCount);
	for (int m = 0; m < kModelCount; ++m)
		_models.push_back(buildModel(kModelFiles[m]));
}

Trees::TreeModel Trees::buildModel(const char* path)
{
	TreeModel model;
	glGenVertexArrays(1, &model.vao);
	glGenVertexArrays(1, &model.shadowVao);
	glGenBuffers(1, &model.vbo);
	glGenBuffers(1, &model.ebo);

	ObjMesh mesh = loadObj(path);
	const size_t vertexCount = mesh.vertices.size() / ObjMesh::kVertexStride;
	const size_t posStride = ObjMesh::kVertexStride * sizeof(float);

	// An LFS pointer stub parses as an empty mesh (tinyobj doesn't error),
	// which would otherwise build zero LODs and crash the draw loop.
	if (mesh.indices.empty())
		throw std::runtime_error(
			std::string("tree mesh has no geometry: ") + path
			+ " - is it a git-lfs pointer? run `git lfs pull`");

	float lo[3] = { 1e30f, 1e30f, 1e30f };
	float hi[3] = { -1e30f, -1e30f, -1e30f };
	for (size_t i = 0; i < vertexCount; ++i)
	{
		const float* p = &mesh.vertices[i * ObjMesh::kVertexStride];
		for (int c = 0; c < 3; ++c)
		{
			lo[c] = std::min(lo[c], p[c]);
			hi[c] = std::max(hi[c], p[c]);
		}
	}
	float diag = 0.0f;
	for (int c = 0; c < 3; ++c)
	{
		model.boundsCenter[c] = 0.5f * (lo[c] + hi[c]);
		float d = hi[c] - lo[c];
		diag += d * d;
	}
	model.boundsRadius = 0.5f * std::sqrt(diag);

	// Sparse mode handles tens of thousands of disconnected leaf objects -
	// simplification reaches the target across components.
	struct LodSpec
	{
		float ratio;
		float error;
		unsigned int options;
	};
	const LodSpec specs[] = {
		{ 0.12f, 0.02f, 0u },
		{ 0.03f, 0.08f, meshopt_SimplifySparse },
	};

	std::vector<unsigned int> allIndices;
	for (const LodSpec& spec : specs)
	{
		size_t target = (size_t)(mesh.indices.size() * spec.ratio);
		std::vector<unsigned int> lodIdx(mesh.indices.size());
		float resultError = 0.0f;
		size_t n = meshopt_simplify(lodIdx.data(), mesh.indices.data(),
									mesh.indices.size(), mesh.vertices.data(),
									vertexCount, posStride, target, spec.error,
									spec.options, &resultError);
		if (n == 0)
			continue;
		lodIdx.resize(n);
		meshopt_optimizeVertexCache(lodIdx.data(), lodIdx.data(), n,
									vertexCount);

		Lod lod;
		lod.indexCount = (GLsizei)n;
		lod.indexByteOffset =
			(GLintptr)(allIndices.size() * sizeof(unsigned int));
		allIndices.insert(allIndices.end(), lodIdx.begin(), lodIdx.end());
		model.lods.push_back(lod);
	}

	// Shadows want silhouette, not detail, so use the mid LOD, fall back if fewer
	// built.
	model.shadowLod = std::min<int>(1, (int)model.lods.size() - 1);

	glBindVertexArray(model.vao); // so the EBO binding records into a real VAO
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float),
				 mesh.vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 allIndices.size() * sizeof(unsigned int), allIndices.data(),
				 GL_STATIC_DRAW);

	return model;
}

void Trees::setupVao(GLuint vao, GLuint vbo, GLuint ebo,
					 GLuint instanceVbo) const
{
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	const GLsizei stride = ObjMesh::kVertexStride * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
						  (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
						  (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, stride,
						  (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(6);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TreeInstance),
						  (void*)offsetof(TreeInstance, pos));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(TreeInstance),
						  (void*)offsetof(TreeInstance, yaw));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(TreeInstance),
						  (void*)offsetof(TreeInstance, scale));
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);
}

void Trees::scatter(const Terrain& terrain)
{
	const int S = Terrain::kHeightmapSize;
	const float worldSize = Terrain::kWorldSize;
	const float half = worldSize * 0.5f;
	const float heightScale = Terrain::kHeightScale;

	std::vector<float> heights((size_t)S * S);
	glGetTextureImage(terrain.heightmap().id(), 0, GL_RED, GL_FLOAT,
					  (GLsizei)(heights.size() * sizeof(float)),
					  heights.data());

	std::vector<float> coverage((size_t)S * S);
	glGetTextureImage(terrain.coverageMask().id(), 0, GL_RED, GL_FLOAT,
					  (GLsizei)(coverage.size() * sizeof(float)),
					  coverage.data());

	const float clearingRadius = 28.0f;
	const float outerRadius = 95.0f;
	const float coverageThreshold = 0.30f;
	const int target = 150;
	const int maxAttempts = 60000;

	// Keep trees spaced apart and throw out any candidate closer than minDist to a
	// tree we already placed. A grid sized so each cell holds at most one tree
	// lets us check only the surrounding cells instead of every tree.
	const float minDist = 5.5f;
	const float minDistSq = minDist * minDist;
	const float cellSize = minDist / 1.41421356f;
	const int gridDim = (int)std::ceil(worldSize / cellSize);
	std::vector<int> grid((size_t)gridDim * gridDim, -1);
	auto cellOf = [&](float w) {
		return std::clamp((int)((w + half) / cellSize), 0, gridDim - 1);
	};

	std::mt19937 rng(2024u);
	std::uniform_real_distribution<float> posDist(-half, half);
	std::uniform_real_distribution<float> yawDist(0.0f, 6.2831853f);
	std::uniform_real_distribution<float> scaleDist(0.8f, 1.5f);
	std::uniform_int_distribution<int> modelDist(0, (int)_models.size() - 1);

	_instances.clear();
	_instances.reserve(target);
	_instanceModel.clear();
	_instanceModel.reserve(target);
	for (int a = 0; a < maxAttempts && (int)_instances.size() < target; ++a)
	{
		float x = posDist(rng);
		float z = posDist(rng);
		float dist = std::sqrt(x * x + z * z);
		if (dist < clearingRadius || dist > outerRadius)
			continue;
		if (sampleHeightField(coverage, S, half, worldSize, x, z)
			< coverageThreshold)
			continue;

		int gx = cellOf(x), gz = cellOf(z);
		bool tooClose = false;
		for (int dz = -2; dz <= 2 && !tooClose; ++dz)
			for (int dx = -2; dx <= 2; ++dx)
			{
				int nx = gx + dx, nz = gz + dz;
				if (nx < 0 || nx >= gridDim || nz < 0 || nz >= gridDim)
					continue;
				int idx = grid[(size_t)nz * gridDim + nx];
				if (idx < 0)
					continue;
				float ddx = _instances[idx].pos[0] - x;
				float ddz = _instances[idx].pos[2] - z;
				if (ddx * ddx + ddz * ddz < minDistSq)
				{
					tooClose = true;
					break;
				}
			}
		if (tooClose)
			continue;

		TreeInstance inst{};
		inst.pos[0] = x;
		inst.pos[1] =
			sampleHeightField(heights, S, half, worldSize, x, z) * heightScale;
		inst.pos[2] = z;
		inst.yaw = yawDist(rng);
		inst.scale = scaleDist(rng);
		grid[(size_t)gz * gridDim + gx] = (int)_instances.size();
		_instances.push_back(inst);
		_instanceModel.push_back(modelDist(rng));
	}
	_instanceCount = (GLsizei)_instances.size();

	// Group instances by model so each model's casters are a contiguous slice
	// the shadow pass can draw with one baseInstance offset.
	{
		std::vector<int> order(_instances.size());
		for (size_t i = 0; i < order.size(); ++i)
			order[i] = (int)i;
		std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
			return _instanceModel[a] < _instanceModel[b];
		});
		std::vector<TreeInstance> sortedInst(_instances.size());
		std::vector<int> sortedModel(_instances.size());
		for (size_t i = 0; i < order.size(); ++i)
		{
			sortedInst[i] = _instances[order[i]];
			sortedModel[i] = _instanceModel[order[i]];
		}
		_instances.swap(sortedInst);
		_instanceModel.swap(sortedModel);
	}

	for (size_t m = 0; m < _models.size(); ++m)
	{
		_models[m].shadowBase = 0;
		_models[m].shadowCount = 0;
	}
	for (size_t i = 0; i < _instanceModel.size(); ++i)
	{
		TreeModel& model = _models[_instanceModel[i]];
		if (model.shadowCount == 0)
			model.shadowBase = (GLint)i;
		model.shadowCount++;
	}

	// Shadow pass draws every caster with a static buffer of all instances. The
	// gbuffer subset is culled per frame, so its buffer is dynamic.
	glGenBuffers(1, &_shadowInstanceVbo);
	glBindBuffer(GL_ARRAY_BUFFER, _shadowInstanceVbo);
	glBufferData(GL_ARRAY_BUFFER, _instances.size() * sizeof(TreeInstance),
				 _instances.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &_instanceVbo);
	glBindBuffer(GL_ARRAY_BUFFER, _instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, _instances.size() * sizeof(TreeInstance),
				 nullptr, GL_DYNAMIC_DRAW);

	glObjectLabel(GL_BUFFER, _shadowInstanceVbo, -1,
				  "Trees shadow instance VBO");
	glObjectLabel(GL_BUFFER, _instanceVbo, -1, "Trees instance VBO");
}

void Trees::draw(const float* mvp) const
{
	// Pull the six planes of the camera's view volume out of the
	// view-projection matrix (left, right, bottom, top, near, far). A point is
	// inside a plane when its dot product with that plane comes out
	// non-negative.
	float planes[6][4];
	for (int i = 0; i < 6; ++i)
	{
		int axis = i >> 1;
		float sign = (i & 1) ? -1.0f : 1.0f;
		for (int c = 0; c < 4; ++c)
			planes[i][c] = mvp[12 + c] + sign * mvp[axis * 4 + c];
		float inv = 1.0f
			/ std::sqrt(planes[i][0] * planes[i][0]
						+ planes[i][1] * planes[i][1]
						+ planes[i][2] * planes[i][2]);
		for (int c = 0; c < 4; ++c)
			planes[i][c] *= inv;
	}

	// Every visible tree draws at full LOD 0 - no distance switching.
	std::vector<std::vector<TreeInstance>> visible(_models.size());
	for (size_t idx = 0; idx < _instances.size(); ++idx)
	{
		const TreeInstance& inst = _instances[idx];
		const TreeModel& model = _models[_instanceModel[idx]];

		// Object-space centre yaw-rotated and scaled to match the vertex
		// shader.
		float c = std::cos(inst.yaw), s = std::sin(inst.yaw);
		float rx = c * model.boundsCenter[0] + s * model.boundsCenter[2];
		float rz = -s * model.boundsCenter[0] + c * model.boundsCenter[2];
		float wx = inst.pos[0] + inst.scale * rx;
		float wy = inst.pos[1] + inst.scale * model.boundsCenter[1];
		float wz = inst.pos[2] + inst.scale * rz;
		float radius = inst.scale * model.boundsRadius;

		bool inView = true;
		for (int i = 0; i < 6 && inView; ++i)
		{
			float d = planes[i][0] * wx + planes[i][1] * wy + planes[i][2] * wz
				+ planes[i][3];
			if (d < -radius)
				inView = false;
		}
		if (inView)
			visible[_instanceModel[idx]].push_back(inst);
	}

	std::vector<TreeInstance> ordered;
	ordered.reserve(_instanceCount);
	std::vector<GLint> base(_models.size());
	std::vector<GLsizei> count(_models.size());
	for (size_t m = 0; m < _models.size(); ++m)
	{
		base[m] = (GLint)ordered.size();
		count[m] = (GLsizei)visible[m].size();
		ordered.insert(ordered.end(), visible[m].begin(), visible[m].end());
	}

	if (ordered.empty())
		return;

	glBindBuffer(GL_ARRAY_BUFFER, _instanceVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ordered.size() * sizeof(TreeInstance),
					ordered.data());

	_shader.use();
	_shader.setUniformMat4("modelViewProjection", mvp);

	for (size_t m = 0; m < _models.size(); ++m)
	{
		const TreeModel& model = _models[m];
		if (count[m] == 0 || model.lods.empty())
			continue;
		glBindVertexArray(model.vao);
		glDrawElementsInstancedBaseInstance(
			GL_TRIANGLES, model.lods[0].indexCount, GL_UNSIGNED_INT,
			(void*)model.lods[0].indexByteOffset, count[m], (GLuint)base[m]);
	}
	glBindVertexArray(0);
}

void Trees::drawDepthOnly(const Shader& shader) const
{
	(void)shader; // matrix is set by the caller, we only supply the geometry

	for (const TreeModel& model : _models)
	{
		if (model.lods.empty() || model.shadowCount == 0)
			continue;
		const Lod& lod = model.lods[model.shadowLod];
		glBindVertexArray(model.shadowVao);
		glDrawElementsInstancedBaseInstance(
			GL_TRIANGLES, lod.indexCount, GL_UNSIGNED_INT,
			(void*)lod.indexByteOffset, model.shadowCount,
			(GLuint)model.shadowBase);
	}
	glBindVertexArray(0);
}
