#include "render/ShadowPass.hh"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "app/RenderContext.hh"
#include "helpers/LookAt.hh"
#include "helpers/Ortho.hh"
#include "helpers/Vec4.hh"
#include "scene/Terrain.hh"
#include "scene/Trees.hh"

ShadowPass::ShadowPass()
	: _fbo("Shadow FBO")
	, _depthTex(kShadowMapSize, kShadowMapSize, GL_DEPTH_COMPONENT32F,
				GL_NEAREST, GL_CLAMP_TO_EDGE, "shadowMap")
	, _shader(SHADER_DIR "/passes/shadow.vert",
			  SHADER_DIR "/passes/shadow.frag")
	, _treeShader(SHADER_DIR "/passes/shadow_tree.vert",
				  SHADER_DIR "/passes/shadow.frag")
{
	_fbo.bind();
	_fbo.attachDepth(_depthTex);
	_fbo.setDrawBuffers(0);

	if (!_fbo.isComplete())
		throw std::runtime_error("Shadow framebuffer is incomplete");

	_fbo.unbind();
}

Matrix4 ShadowPass::buildShadowMatrix(const RenderContext& ctx) const
{
	const float* sunDir = ctx.sunDir;
	const float maxDist = ctx.style.shadowMaxDistance;
	const float sunDistance = 300.0f;

	Matrix4 proj, view;
	for (int i = 0; i < 16; ++i)
	{
		proj.m[i] = ctx.projMatrix[i];
		view.m[i] = ctx.viewMatrix[i];
	}
	Matrix4 invViewProj = inverse(proj * view);

	Vec4 worldCorners[8];
	int idx = 0;
	for (int zi = 0; zi < 2; ++zi)
		for (int yi = -1; yi <= 1; yi += 2)
			for (int xi = -1; xi <= 1; xi += 2)
			{
				float ndcZ = (zi == 0) ? -1.0f : 1.0f;
				Vec4 h = invViewProj * Vec4((float)xi, (float)yi, ndcZ, 1.0f);
				worldCorners[idx++] = h * (1.0f / h.w);
			}

	// Clamp each far corner toward its near corner so the slice is at most
	// maxDist deep, distant corners would blow the ortho out and waste texels.
	for (int i = 0; i < 4; ++i)
	{
		Vec4 nearC = worldCorners[i];
		Vec4 ray = worldCorners[i + 4] - nearC;
		float len = std::sqrt(ray.x * ray.x + ray.y * ray.y + ray.z * ray.z);
		if (len > maxDist)
			worldCorners[i + 4] = nearC + ray * (maxDist / len);
	}

	Vec4 lightDir(sunDir[0], sunDir[1], sunDir[2], 0.0f);
	Vec4 up(0.0f, 1.0f, 0.0f, 0.0f);
	if (std::abs(lightDir.dot(up)) > 0.99f)
		up = Vec4(1.0f, 0.0f, 0.0f, 0.0f);

	Vec4 center(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 8; ++i)
		center += worldCorners[i];
	center *= (1.0f / 8.0f);

	Vec4 eye = center + lightDir * sunDistance;
	Matrix4 lightView = lookAt(eye.x, eye.y, eye.z, center.x, center.y,
							   center.z, up.x, up.y, up.z);

	Vec4 c0 = lightView * worldCorners[0];
	float minX = c0.x, maxX = c0.x;
	float minY = c0.y, maxY = c0.y;
	float minZ = c0.z, maxZ = c0.z;
	for (int i = 1; i < 8; ++i)
	{
		Vec4 c = lightView * worldCorners[i];
		minX = std::min(minX, c.x);
		maxX = std::max(maxX, c.x);
		minY = std::min(minY, c.y);
		maxY = std::max(maxY, c.y);
		minZ = std::min(minZ, c.z);
		maxZ = std::max(maxZ, c.z);
	}

	float texelX = (maxX - minX) / (float)kShadowMapSize;
	float texelY = (maxY - minY) / (float)kShadowMapSize;
	minX = std::floor(minX / texelX) * texelX;
	minY = std::floor(minY / texelY) * texelY;

	float znear = -maxZ - 10.0f;
	float zfar = -minZ + 10.0f;

	Matrix4 lightProj = ortho(minX, maxX, minY, maxY, znear, zfar);
	return lightProj * lightView;
}

void ShadowPass::render(const Terrain& terrain, const Trees& trees,
						const RenderContext& ctx)
{
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	_fbo.bind();
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 8.0f);

	// Terrain is single-sided heightmap - culling fronts drops the casting
	// surface. Slope-scaled bias handles acne instead.
	_lightSpaceMat = buildShadowMatrix(ctx);
	_shader.use();
	_shader.setUniformMat4("uShadowMatrix", _lightSpaceMat.m);

	terrain.drawDepthOnly(_shader);

	// Trunks are closed - front-face cull puts back depth in the map, no
	// self-shadow on lit faces
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	_treeShader.use();
	_treeShader.setUniformMat4("uShadowMatrix", _lightSpaceMat.m);
	trees.drawDepthOnly(_treeShader);
	glDisable(GL_CULL_FACE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0f, 0.0f);
	_fbo.unbind();
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2],
			   prevViewport[3]);
}
