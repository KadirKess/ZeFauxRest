#include "render/CloudPass.hh"

#include <glad/glad.h>

#include "app/RenderContext.hh"
#include "helpers/Matrix4.hh"
#include "render/GBuffer.hh"

CloudPass::CloudPass(int width, int height)
	: _cloudRGBA(width / 2, height / 2, GL_RGBA16F, GL_LINEAR, GL_CLAMP_TO_EDGE,
				 "cloudRGBA")
	, _shader(SHADER_DIR "/passes/clouds.comp")
{
	glObjectLabel(GL_PROGRAM, _shader.id(), -1, "Cloud shader");
}

void CloudPass::render(const GBuffer& gbuffer, const RenderContext& ctx,
					   const Texture& blueNoise)
{
	const StyleParams& style = ctx.style;

	Matrix4 proj, view;
	for (int i = 0; i < 16; ++i)
	{
		proj.m[i] = ctx.projMatrix[i];
		view.m[i] = ctx.viewMatrix[i];
	}
	Matrix4 invViewProj = inverse(proj * view);

	// Camera world position = -R^T * t from the row-major view matrix.
	const float* v = view.m;
	float camPos[3] = {
		-(v[0] * v[3] + v[4] * v[7] + v[8] * v[11]),
		-(v[1] * v[3] + v[5] * v[7] + v[9] * v[11]),
		-(v[2] * v[3] + v[6] * v[7] + v[10] * v[11]),
	};

	gbuffer.depth().bindToUnit(0);
	_noise.shape().bindToUnit(1);
	_noise.detail().bindToUnit(2);
	blueNoise.bindToUnit(3);
	_cloudRGBA.bindAsImage(0, GL_WRITE_ONLY);

	_shader.use();
	_shader.setUniform1i("gDepth", 0);
	_shader.setUniform1i("shapeNoise", 1);
	_shader.setUniform1i("detailNoise", 2);
	_shader.setUniform1i("blueNoise", 3);
	_shader.setUniformMat4("invViewProj", invViewProj.m);
	_shader.setUniformVec3("camPos", camPos);
	_shader.setUniformVec3("sunColor", style.sunColor);
	_shader.setUniformVec3("cloudAmbientColor", style.cloudAmbientColor);
	_shader.setUniformVec3("sunDir", ctx.sunDir);
	_shader.setUniform1f("cloudCoverage", style.cloudCoverage);
	_shader.setUniform1f("cloudDensityScale", style.cloudDensityScale);
	_shader.setUniform1f("cloudDetailWeight", style.cloudDetailWeight);
	_shader.setUniform1f("cloudPhaseG", style.cloudPhaseG);

	GLuint groupsX = (_cloudRGBA.width() + 15) / 16;
	GLuint groupsY = (_cloudRGBA.height() + 15) / 16;
	glDispatchCompute(groupsX, groupsY, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT);
}
