#pragma once

#include "gl/Framebuffer.hh"
#include "gl/Shader.hh"
#include "gl/Texture.hh"

struct RenderContext;
struct SkyModel;

// bilateral upsample uses depth to avoid silhouette halos on clouds/godrays
class CompositePass
{
private:
	Texture _hdrComposite;
	Framebuffer _fbo;
	Shader _shader;
	GLuint _vao;
	int _width;
	int _height;

public:
	CompositePass(int width, int height);
	~CompositePass();

	CompositePass(const CompositePass&) = delete;
	CompositePass& operator=(const CompositePass&) = delete;

	void render(const Texture& litScene, const Texture& cloudRGBA,
				const Texture& godrayRGB, const Texture& fullDepth,
				const Texture& minDepthHalf, const SkyModel& sky,
				const RenderContext& ctx);

	const Texture& hdrComposite() const
	{
		return _hdrComposite;
	}
};
