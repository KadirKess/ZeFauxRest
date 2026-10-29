#pragma once

#include "gl/Shader.hh"
#include "gl/Texture.hh"

class GBuffer;
struct RenderContext;

// RGBA16F because RGB16F isn't a legal imageStore format
class GodrayPass
{
private:
	Texture _godrayRGB;
	Shader _shader;

public:
	GodrayPass(int width, int height);

	GodrayPass(const GodrayPass&) = delete;
	GodrayPass& operator=(const GodrayPass&) = delete;

	void render(const GBuffer& gbuffer, const Texture& shadowMap,
				const RenderContext& ctx, const Texture& blueNoise);

	const Texture& godrayRGB() const
	{
		return _godrayRGB;
	}
};
