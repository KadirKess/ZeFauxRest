#pragma once

#include "gl/Shader.hh"
#include "gl/Texture.hh"

class GBuffer;
struct RenderContext;

class DeferredLightingPass
{
private:
	Texture _litScene;
	Shader _shader;

public:
	DeferredLightingPass(int width, int height);

	DeferredLightingPass(const DeferredLightingPass&) = delete;
	DeferredLightingPass& operator=(const DeferredLightingPass&) = delete;

	void render(const GBuffer& gbuffer, const Texture& shadowMap,
				const RenderContext& ctx);

	const Texture& litScene() const
	{
		return _litScene;
	}
};
