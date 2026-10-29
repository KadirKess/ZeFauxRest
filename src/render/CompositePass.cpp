#include "render/CompositePass.hh"

#include <glad/glad.h>
#include <stdexcept>

#include "app/RenderContext.hh"
#include "helpers/Matrix4.hh"
#include "render/SkyModel.hh"

CompositePass::CompositePass(int width, int height)
	: _hdrComposite(width, height, GL_RGB16F, GL_LINEAR, GL_CLAMP_TO_EDGE,
					"hdrComposite")
	, _fbo("Composite FBO")
	, _shader(SHADER_DIR "/passes/fullscreen_uv.vert",
			  SHADER_DIR "/passes/composite.frag")
	, _vao(0)
	, _width(width)
	, _height(height)
{
	_fbo.bind();
	_fbo.attachColor(_hdrComposite, 0);
	_fbo.setDrawBuffers(1);
	if (!_fbo.isComplete())
		throw std::runtime_error("Composite framebuffer is incomplete");
	_fbo.unbind();

	glCreateVertexArrays(1, &_vao);
	glObjectLabel(GL_VERTEX_ARRAY, _vao, -1, "Composite VAO");
	glObjectLabel(GL_PROGRAM, _shader.id(), -1, "Composite shader");
}

CompositePass::~CompositePass()
{
	glDeleteVertexArrays(1, &_vao);
}

void CompositePass::render(const Texture& litScene, const Texture& cloudRGBA,
						   const Texture& godrayRGB, const Texture& fullDepth,
						   const Texture& minDepthHalf, const SkyModel& sky,
						   const RenderContext& ctx)
{
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

	_fbo.bind();
	glViewport(0, 0, _width, _height);

	litScene.bindToUnit(0);
	cloudRGBA.bindToUnit(1);
	godrayRGB.bindToUnit(2);
	fullDepth.bindToUnit(3);
	minDepthHalf.bindToUnit(4);

	glDisable(GL_DEPTH_TEST);

	_shader.use();
	_shader.setUniform1i("litScene", 0);
	_shader.setUniform1i("cloudRGBA", 1);
	_shader.setUniform1i("godrayRGB", 2);
	_shader.setUniform1i("gDepth", 3);
	_shader.setUniform1i("minDepthHalf", 4);
	_shader.setUniformMat4("invViewProj", invViewProj.m);
	_shader.setUniformVec3("camPos", camPos);

	// Aerial perspective shares the sky's coefficients (cooked once per frame)
	// so the haze color matches sampleSky() in the sky pass - no horizon seam.
	_shader.setUniformVec3("sunDir", ctx.sunDir);
	sky.upload(_shader);
	_shader.setUniform1f("apStrength", ctx.style.aerialPerspectiveStrength);
	_shader.setUniform1f("apStart", ctx.style.aerialPerspectiveStart);
	_shader.setUniform1f("apFalloff", ctx.style.aerialPerspectiveFalloff);
	_shader.setUniform1f("apCanopyFade", ctx.style.aerialPerspectiveCanopyFade);

	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	_fbo.unbind();
}
