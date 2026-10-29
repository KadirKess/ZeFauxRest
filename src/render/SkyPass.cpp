#include "render/SkyPass.hh"

#include <cmath>
#include <glad/glad.h>

#include "app/RenderContext.hh"
#include "gl/Texture.hh"
#include "helpers/Matrix4.hh"
#include "render/GBuffer.hh"
#include "render/SkyModel.hh"

SkyPass::SkyPass(int width, int height)
	: _fbo("Sky FBO")
	, _shader(SHADER_DIR "/passes/sky.vert", SHADER_DIR "/passes/sky.frag",
			  { "SKY_MODEL SKY_HOSEK_WILKIE" })
	, _vao(0)
	, _width(width)
	, _height(height)
{
	glCreateVertexArrays(1, &_vao);
	glObjectLabel(GL_VERTEX_ARRAY, _vao, -1, "Sky VAO");
	glObjectLabel(GL_PROGRAM, _shader.id(), -1, "Sky shader");
}

SkyPass::~SkyPass()
{
	glDeleteVertexArrays(1, &_vao);
}

void SkyPass::render(const Texture& litScene, const GBuffer& gbuffer,
					 const SkyModel& sky, const RenderContext& ctx)
{
	Matrix4 proj, view;
	for (int i = 0; i < 16; ++i)
	{
		proj.m[i] = ctx.projMatrix[i];
		view.m[i] = ctx.viewMatrix[i];
	}
	Matrix4 invViewProj = inverse(proj * view);

	_fbo.bind();
	_fbo.attachColor(litScene, 0);
	_fbo.setDrawBuffers(1);
	glViewport(0, 0, _width, _height);

	gbuffer.depth().bindToUnit(0);

	glDisable(GL_DEPTH_TEST);

	_shader.use();
	_shader.setUniformMat4("invViewProj", invViewProj.m);
	_shader.setUniformVec3("sunDir", ctx.sunDir);

	sky.upload(_shader);

	// ~1.7 deg angular radius (cos(0.03))
	float sunDiscColor[3] = { ctx.style.sunColor[0] * ctx.style.sunIntensity,
							  ctx.style.sunColor[1] * ctx.style.sunIntensity,
							  ctx.style.sunColor[2] * ctx.style.sunIntensity };
	_shader.setUniformVec3("sunDiscColor", sunDiscColor);
	_shader.setUniform1f("sunDiscCos", std::cos(0.03f));

	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	_fbo.unbind();
}
