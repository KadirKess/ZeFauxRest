#pragma once

#include "gl/Mesh.hh"
#include "gl/Shader.hh"

class Terrain;
class Texture;
struct RenderContext;

struct GrassInstance
{
	float pos[3];
	float yaw;
	float heightScale;
	float colorSeed;
};

class Grass
{
private:
	Mesh _mesh;
	Shader _shader;
	GLuint _instanceVbo;
	GLsizei _instanceCount;
	GLsizei _drawIndexCount;

	static Mesh buildBlade();
	void scatter(const Terrain& terrain);

public:
	explicit Grass(const Terrain& terrain);
	~Grass();

	Grass(const Grass&) = delete;
	Grass& operator=(const Grass&) = delete;

	void draw(const float* mvp, const RenderContext& ctx,
			  const Texture& blueNoise) const;
};
