#include "render/GodrayPass.hh"

#include <glad/glad.h>

#include "app/RenderContext.hh"
#include "helpers/Matrix4.hh"
#include "render/GBuffer.hh"

GodrayPass::GodrayPass(int width, int height)
	: _godrayRGB(width / 2, height / 2, GL_RGBA16F, GL_LINEAR, GL_CLAMP_TO_EDGE,
				 "godrayRGB")
	, _shader(SHADER_DIR "/passes/godrays.comp")
{
	glObjectLabel(GL_PROGRAM, _shader.id(), -1, "Godray shader");
}

void GodrayPass::render(const GBuffer& gbuffer, const Texture& shadowMap,
						const RenderContext& ctx, const Texture& blueNoise)
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

	gbuffer.depth().bindToUnit(0);
	shadowMap.bindToUnit(1);
	blueNoise.bindToUnit(2);
	_godrayRGB.bindAsImage(0, GL_WRITE_ONLY);

	_shader.use();
	_shader.setUniform1i("gDepth", 0);
	_shader.setUniform1i("shadowMap", 1);
	_shader.setUniform1i("blueNoise", 2);
	_shader.setUniformMat4("invViewProj", invViewProj.m);
	_shader.setUniformVec3("camPos", camPos);
	_shader.setUniformVec3("lightColor", style.godrayColor);
	_shader.setUniformVec3("sunDir", ctx.sunDir);
	_shader.setUniform1f("godrayIntensity", style.godrayIntensity);
	_shader.setUniform1f("godrayPhaseG", style.godrayPhaseG);
	_shader.setUniform1f("godrayMaxDistance", style.godrayMaxDistance);
	_shader.setUniform1f("godrayDensity", style.godrayDensity);

	GLuint groupsX = (_godrayRGB.width() + 15) / 16;
	GLuint groupsY = (_godrayRGB.height() + 15) / 16;
	glDispatchCompute(groupsX, groupsY, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT);
}
