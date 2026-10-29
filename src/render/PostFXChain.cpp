#include "render/PostFXChain.hh"

#include <algorithm>
#include <glad/glad.h>
#include <stdexcept>

#include "app/RenderContext.hh"

PostFXChain::PostFXChain(int width, int height)
	: _tonemapped(width, height, GL_RGB16F, GL_LINEAR, GL_CLAMP_TO_EDGE,
				  "tonemapped")
	, _fbo("PostFX FBO")
	, _graded(width, height, GL_RGB16F, GL_LINEAR, GL_CLAMP_TO_EDGE, "graded")
	, _gradeFbo("Color grade FBO")
	, _bloomResult(std::max(1, width / 2), std::max(1, height / 2), GL_RGB16F,
				   GL_LINEAR, GL_CLAMP_TO_EDGE, "bloom result")
	, _bloomResultFbo("Bloom result FBO")
	, _bloomPrefilter(SHADER_DIR "/passes/fullscreen_uv.vert",
					  SHADER_DIR "/passes/bloom_prefilter.frag")
	, _bloomDown(SHADER_DIR "/passes/fullscreen_uv.vert",
				 SHADER_DIR "/passes/bloom_down.frag")
	, _bloomUp(SHADER_DIR "/passes/fullscreen_uv.vert",
			   SHADER_DIR "/passes/bloom_up.frag")
	, _tonemap(SHADER_DIR "/passes/fullscreen_uv.vert",
			   SHADER_DIR "/passes/tonemap.frag")
	, _colorGrade(SHADER_DIR "/passes/fullscreen_uv.vert",
				  SHADER_DIR "/passes/color_grade.frag")
	, _vignette(SHADER_DIR "/passes/fullscreen_uv.vert",
				SHADER_DIR "/passes/vignette.frag")
	, _vao(0)
	, _width(width)
	, _height(height)
{
	_fbo.bind();
	_fbo.attachColor(_tonemapped, 0);
	_fbo.setDrawBuffers(1);
	if (!_fbo.isComplete())
		throw std::runtime_error("PostFX framebuffer is incomplete");
	_fbo.unbind();

	_gradeFbo.bind();
	_gradeFbo.attachColor(_graded, 0);
	_gradeFbo.setDrawBuffers(1);
	if (!_gradeFbo.isComplete())
		throw std::runtime_error("Color grade framebuffer is incomplete");
	_gradeFbo.unbind();

	// RGB16F so the bright extract keeps HDR headroom - RGB8 clips the sun disc
	_bloomMips.reserve(kBloomMips);
	_bloomFbos.reserve(kBloomMips);
	for (int i = 0; i < kBloomMips; ++i)
	{
		int w = std::max(1, width >> (i + 1));
		int h = std::max(1, height >> (i + 1));
		_bloomMips.emplace_back(w, h, GL_RGB16F, GL_LINEAR, GL_CLAMP_TO_EDGE,
								"bloom mip");
		_bloomFbos.emplace_back("Bloom mip FBO");

		_bloomFbos[i].bind();
		_bloomFbos[i].attachColor(_bloomMips[i], 0);
		_bloomFbos[i].setDrawBuffers(1);
		if (!_bloomFbos[i].isComplete())
			throw std::runtime_error("Bloom mip framebuffer is incomplete");
		_bloomFbos[i].unbind();
	}

	_bloomResultFbo.bind();
	_bloomResultFbo.attachColor(_bloomResult, 0);
	_bloomResultFbo.setDrawBuffers(1);
	if (!_bloomResultFbo.isComplete())
		throw std::runtime_error("Bloom result framebuffer is incomplete");
	_bloomResultFbo.unbind();

	glCreateVertexArrays(1, &_vao);
	glObjectLabel(GL_VERTEX_ARRAY, _vao, -1, "PostFX VAO");
	glObjectLabel(GL_PROGRAM, _bloomPrefilter.id(), -1,
				  "Bloom prefilter shader");
	glObjectLabel(GL_PROGRAM, _bloomDown.id(), -1, "Bloom down shader");
	glObjectLabel(GL_PROGRAM, _bloomUp.id(), -1, "Bloom up shader");
	glObjectLabel(GL_PROGRAM, _tonemap.id(), -1, "Tonemap shader");
	glObjectLabel(GL_PROGRAM, _colorGrade.id(), -1, "Color grade shader");
	glObjectLabel(GL_PROGRAM, _vignette.id(), -1, "Vignette shader");
}

PostFXChain::~PostFXChain()
{
	glDeleteVertexArrays(1, &_vao);
}

void PostFXChain::renderBloom(const Texture& hdrComposite,
							  const RenderContext& ctx)
{
	_bloomFbos[0].bind();
	glViewport(0, 0, _bloomMips[0].width(), _bloomMips[0].height());
	hdrComposite.bindToUnit(0);
	_bloomPrefilter.use();
	_bloomPrefilter.setUniform1i("source", 0);
	float fullTexel[2] = { 1.0f / _width, 1.0f / _height };
	_bloomPrefilter.setUniformVec2("srcTexel", fullTexel);
	_bloomPrefilter.setUniform1f("threshold", ctx.style.bloomThreshold);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	_bloomDown.use();
	_bloomDown.setUniform1i("source", 0);
	for (int i = 1; i < kBloomMips; ++i)
	{
		_bloomFbos[i].bind();
		glViewport(0, 0, _bloomMips[i].width(), _bloomMips[i].height());
		_bloomMips[i - 1].bindToUnit(0);
		float texel[2] = { 1.0f / _bloomMips[i - 1].width(),
						   1.0f / _bloomMips[i - 1].height() };
		_bloomDown.setUniformVec2("srcTexel", texel);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	// Front-loaded weights keep the halo tight - reordering toward larger mips
	// smears the glow
	static const float weights[kBloomMips] = { 0.5f, 0.25f, 0.15f, 0.1f };

	_bloomResultFbo.bind();
	glViewport(0, 0, _bloomResult.width(), _bloomResult.height());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	_bloomUp.use();
	_bloomUp.setUniform1i("source", 0);
	for (int i = 0; i < kBloomMips; ++i)
	{
		_bloomMips[i].bindToUnit(0);
		float texel[2] = { 1.0f / _bloomMips[i].width(),
						   1.0f / _bloomMips[i].height() };
		_bloomUp.setUniformVec2("srcTexel", texel);
		_bloomUp.setUniform1f("weight", weights[i]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	glDisable(GL_BLEND);
}

void PostFXChain::render(const Texture& hdrComposite, const RenderContext& ctx)
{
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(_vao);

	renderBloom(hdrComposite, ctx);

	_fbo.bind();
	glViewport(0, 0, _width, _height);
	hdrComposite.bindToUnit(0);
	_bloomResult.bindToUnit(1);
	_tonemap.use();
	_tonemap.setUniform1i("hdrComposite", 0);
	_tonemap.setUniform1i("bloom", 1);
	_tonemap.setUniform1f("bloomStrength", ctx.style.bloomStrength);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	_fbo.unbind();

	_gradeFbo.bind();
	glViewport(0, 0, _width, _height);
	_tonemapped.bindToUnit(0);
	_colorGrade.use();
	_colorGrade.setUniform1i("tonemapped", 0);
	_colorGrade.setUniformVec3("gradeLift", ctx.style.gradeLift);
	_colorGrade.setUniformVec3("gradeGamma", ctx.style.gradeGamma);
	_colorGrade.setUniformVec3("gradeGain", ctx.style.gradeGain);
	_colorGrade.setUniformVec3("highlightTint", ctx.style.highlightTint);
	_colorGrade.setUniformVec3("shadowTint", ctx.style.shadowTint);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	_gradeFbo.unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _width, _height);
	_graded.bindToUnit(0);
	_vignette.use();
	_vignette.setUniform1i("tonemapped", 0);
	_vignette.setUniform1f("vignetteStrength", ctx.style.vignetteStrength);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}
