#pragma once

#include "gl/Shader.hh"
#include "gl/Texture.hh"
#include "render/CloudNoise.hh"

class GBuffer;
struct RenderContext;

// a = transmittance toward camera (1 = clear sky)
class CloudPass
{
private:
	Texture _cloudRGBA;
	Shader _shader;
	CloudNoise _noise;

public:
	CloudPass(int width, int height);

	CloudPass(const CloudPass&) = delete;
	CloudPass& operator=(const CloudPass&) = delete;

	void render(const GBuffer& gbuffer, const RenderContext& ctx,
				const Texture& blueNoise);

	const Texture& cloudRGBA() const
	{
		return _cloudRGBA;
	}
};
