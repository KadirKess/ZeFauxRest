#include "render/DeferredLightingPass.hh"

#include <glad/glad.h>

#include "app/RenderContext.hh"
#include "helpers/Matrix4.hh"
#include "render/GBuffer.hh"

DeferredLightingPass::DeferredLightingPass(int width, int height)
	: _litScene(width, height, GL_RGBA16F, GL_NEAREST, GL_CLAMP_TO_EDGE,
				"litScene")
	, _shader(SHADER_DIR "/passes/deferred_lighting.comp")
{}

void DeferredLightingPass::render(const GBuffer& gbuffer,
								  const Texture& shadowMap,
								  const RenderContext& ctx)
{
	const StyleParams& style = ctx.style;

	Matrix4 proj, view;
	for (int i = 0; i < 16; ++i)
	{
		proj.m[i] = ctx.projMatrix[i];
		view.m[i] = ctx.viewMatrix[i];
	}
	Matrix4 invViewProj = inverse(proj * view);

	const float* v = view.m;
	float camPos[3] = {
		-(v[0] * v[3] + v[4] * v[7] + v[8] * v[11]),
		-(v[1] * v[3] + v[5] * v[7] + v[9] * v[11]),
		-(v[2] * v[3] + v[6] * v[7] + v[10] * v[11]),
	};

	gbuffer.albedo().bindToUnit(0);
	gbuffer.normal().bindToUnit(1);
	gbuffer.depth().bindToUnit(2);
	shadowMap.bindToUnit(3);
	_litScene.bindAsImage(0, GL_WRITE_ONLY);

	_shader.use();
	_shader.setUniformVec3("sunDir", ctx.sunDir);
	_shader.setUniformVec3("sunColor", style.sunColor);
	_shader.setUniform1f("sunIntensity", style.sunIntensity);
	_shader.setUniformVec3("skyAmbientColor", style.skyAmbientColor);
	_shader.setUniform1f("shadowStrength", style.shadowStrength);
	_shader.setUniform1f("specularStrength", style.specularStrength);
	_shader.setUniform1f("foliageTranslucency", style.foliageTranslucency);
	_shader.setUniformMat4("invViewProj", invViewProj.m);
	_shader.setUniformVec3("camPos", camPos);

	GLuint groupsX = (_litScene.width() + 15) / 16;
	GLuint groupsY = (_litScene.height() + 15) / 16;
	glDispatchCompute(groupsX, groupsY, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT
					| GL_FRAMEBUFFER_BARRIER_BIT);
}
