#pragma once

#include "gl/Framebuffer.hh"
#include "gl/Shader.hh"
#include "gl/Texture.hh"
#include "helpers/Matrix4.hh"

class Terrain;
class Trees;
struct RenderContext;

class ShadowPass
{
private:
	Framebuffer _fbo;
	Texture _depthTex;
	Shader _shader;
	Shader _treeShader; // instanced depth path for tree casters
	Matrix4 _lightSpaceMat;

public:
	static constexpr int kShadowMapSize = 2048;

	ShadowPass();

	ShadowPass(const ShadowPass&) = delete;
	ShadowPass& operator=(const ShadowPass&) = delete;

	// needs camera view/proj to fit the frustum, not just the sun direction
	Matrix4 buildShadowMatrix(const RenderContext& ctx) const;

	void render(const Terrain& terrain, const Trees& trees,
				const RenderContext& ctx);

	const Texture& depthTexture() const
	{
		return _depthTex;
	}

	const Matrix4& lightSpaceMatrix() const
	{
		return _lightSpaceMat;
	}
};
