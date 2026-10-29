#pragma once

#include <glad/glad.h>
#include <vector>

#include "gl/Shader.hh"

class Terrain;

struct TreeInstance
{
	float pos[3];
	float yaw;
	float scale;
};

// One mesh per distinct tree model, each with its own meshopt LOD chain
// (G-buffer draws LOD 0, shadows a cheaper silhouette).
class Trees
{
private:
	struct Lod
	{
		GLsizei indexCount = 0;
		GLintptr indexByteOffset = 0;
	};

	// LODs are index ranges concatenated into this model's single EBO.
	struct TreeModel
	{
		GLuint vao = 0;
		GLuint shadowVao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;
		std::vector<Lod> lods; // index 0 = highest detail
		int shadowLod = 0;
		float boundsCenter[3] = { 0.0f, 0.0f, 0.0f };
		float boundsRadius = 0.0f;
		// Contiguous slice of the static shadow buffer (instances grouped by
		// model).
		GLint shadowBase = 0;
		GLsizei shadowCount = 0;
	};

	std::vector<TreeModel> _models;

	GLuint _instanceVbo; // dynamic, rewritten each frame after culling
	GLuint _shadowInstanceVbo; // static, every instance, grouped by model

	std::vector<TreeInstance>
		_instances; // CPU copy, kept for per-frame culling
	std::vector<int> _instanceModel;
	GLsizei _instanceCount;

	Shader _shader;

	TreeModel buildModel(const char* path);
	void buildModels();
	void scatter(const Terrain& terrain);
	void setupVao(GLuint vao, GLuint vbo, GLuint ebo, GLuint instanceVbo) const;

public:
	explicit Trees(const Terrain& terrain);
	~Trees();

	Trees(const Trees&) = delete;
	Trees& operator=(const Trees&) = delete;

	void draw(const float* mvp) const;
	void drawDepthOnly(const Shader& shader) const;
};
