#pragma once

#include "gl/Framebuffer.hh"
#include "gl/Shader.hh"

class Texture;
class GBuffer;
struct RenderContext;
struct SkyModel;

// does not own litScene - borrows it and writes only depth==far pixels so
// deferred lighting skips them
class SkyPass
{
private:
	Framebuffer _fbo;
	Shader _shader;
	GLuint _vao;
	int _width;
	int _height;

public:
	SkyPass(int width, int height);
	~SkyPass();

	SkyPass(const SkyPass&) = delete;
	SkyPass& operator=(const SkyPass&) = delete;

	void render(const Texture& litScene, const GBuffer& gbuffer,
				const SkyModel& sky, const RenderContext& ctx);
};
