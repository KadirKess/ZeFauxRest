#pragma once

#include <vector>

#include "gl/Framebuffer.hh"
#include "gl/Shader.hh"
#include "gl/Texture.hh"

struct RenderContext;

// Hable not ACES - ACES rotates the grass greens toward yellow
// full res 16F throughout, vignette encodes sRGB on the way out
class PostFXChain
{
private:
	static constexpr int kBloomMips = 4;

	Texture _tonemapped;
	Framebuffer _fbo;

	Texture _graded;
	Framebuffer _gradeFbo;

	std::vector<Texture> _bloomMips;
	std::vector<Framebuffer> _bloomFbos;
	Texture _bloomResult;
	Framebuffer _bloomResultFbo;

	Shader _bloomPrefilter;
	Shader _bloomDown;
	Shader _bloomUp;
	Shader _tonemap;
	Shader _colorGrade;
	Shader _vignette;
	GLuint _vao;
	int _width;
	int _height;

	void renderBloom(const Texture& hdrComposite, const RenderContext& ctx);

public:
	PostFXChain(int width, int height);
	~PostFXChain();

	PostFXChain(const PostFXChain&) = delete;
	PostFXChain& operator=(const PostFXChain&) = delete;

	void render(const Texture& hdrComposite, const RenderContext& ctx);
};
