#pragma once

#include "app/RenderContext.hh"
#include "render/BlueNoise.hh"
#include "render/CloudPass.hh"
#include "render/CompositePass.hh"
#include "render/DeferredLightingPass.hh"
#include "render/DepthDownsamplePass.hh"
#include "render/GBuffer.hh"
#include "render/GodrayPass.hh"
#include "render/PostFXChain.hh"
#include "render/ShadowPass.hh"
#include "render/ShadowUniforms.hh"
#include "render/SkyModel.hh"
#include "render/SkyPass.hh"
#include "render/WindUniforms.hh"
#include "scene/Grass.hh"
#include "scene/Terrain.hh"
#include "scene/Trees.hh"

class RenderGraph
{
private:
	GBuffer _gbuffer;
	ShadowPass _shadow;
	DeferredLightingPass _lighting;
	Terrain _terrain;
	Grass _grass;
	Trees _trees;
	SkyPass _sky;
	CloudPass _cloud;
	GodrayPass _godray;
	DepthDownsamplePass _depthDownsample;
	CompositePass _composite;
	PostFXChain _postFX;
	BlueNoise _blueNoise;
	WindUniforms _wind;
	ShadowUniforms _shadowUniforms;
	RenderContext _ctx;
	SkyModel _skyModel;
	float _sunDir[3];

	void shadowPass();
	void gBufferPass();
	void skyPass();
	void deferredLightingPass();
	void cloudPass();
	void godrayPass();
	void depthDownsamplePass();
	void compositePass();
	void postFXPass();

public:
	RenderGraph(int width, int height);

	void execute(const float* viewMatrix, const float* projMatrix, float time);

	// Live-tuning handles for the debug UI, picked up on the next frame.
	StyleParams& style()
	{
		return _ctx.style;
	}

	float* sunDir()
	{
		return _sunDir;
	}

	const GBuffer& gbuffer() const
	{
		return _gbuffer;
	}

	const Texture& shadowMap() const
	{
		return _shadow.depthTexture();
	}

	const Texture& litScene() const
	{
		return _lighting.litScene();
	}

	const Texture& cloudRGBA() const
	{
		return _cloud.cloudRGBA();
	}

	const Texture& godrayRGB() const
	{
		return _godray.godrayRGB();
	}

	const Texture& hdrComposite() const
	{
		return _composite.hdrComposite();
	}
};
