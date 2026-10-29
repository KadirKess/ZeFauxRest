#include "render/RenderGraph.hh"

#include <cmath>
#include <glad/glad.h>

#include "helpers/Matrix4.hh"

RenderGraph::RenderGraph(int width, int height)
	: _gbuffer(width, height)
	, _shadow()
	, _lighting(width, height)
	, _terrain()
	, _grass(_terrain)
	, _trees(_terrain)
	, _sky(width, height)
	, _cloud(width, height)
	, _godray(width, height)
	, _depthDownsample(width, height)
	, _composite(width, height)
	, _postFX(width, height)
	, _blueNoise()
	, _wind()
	, _shadowUniforms()
	, _ctx()
{
	float sx = 0.3f, sy = 0.6f, sz = 0.5f;
	float len = std::sqrt(sx * sx + sy * sy + sz * sz);
	_sunDir[0] = sx / len;
	_sunDir[1] = sy / len;
	_sunDir[2] = sz / len;
}

void RenderGraph::execute(const float* viewMatrix, const float* projMatrix,
						  float time)
{
	_ctx.viewMatrix = viewMatrix;
	_ctx.projMatrix = projMatrix;
	_ctx.sunDir[0] = _sunDir[0];
	_ctx.sunDir[1] = _sunDir[1];
	_ctx.sunDir[2] = _sunDir[2];
	_ctx.time = time;
	_ctx.windStrength = _ctx.style.grassWindStrength;
	_ctx.windGustFrequency = _ctx.style.grassWindGustiness;

	_wind.update(_ctx);

	// Cook the Hosek-Wilkie coefficients once per frame before any sampleSky()
	// pass, sky and aerial perspective share them so AP's fade stays seam-free.
	float sunElevRad =
		std::asin(std::fmax(-1.0f, std::fmin(1.0f, _ctx.sunDir[1])));
	_skyModel.compute(_ctx.style.turbidity, sunElevRad,
					  _ctx.style.skyRadianceScale);

	shadowPass();
	_shadowUniforms.update(_shadow.lightSpaceMatrix());

	gBufferPass();
	skyPass();
	deferredLightingPass();
	cloudPass();
	godrayPass();
	depthDownsamplePass();
	compositePass();
	postFXPass();
}

void RenderGraph::shadowPass()
{
	_shadow.render(_terrain, _trees, _ctx);
}

void RenderGraph::gBufferPass()
{
	// proj and view are row-major, so multiply as proj * view.
	Matrix4 proj, view;
	for (int i = 0; i < 16; ++i)
	{
		proj.m[i] = _ctx.projMatrix[i];
		view.m[i] = _ctx.viewMatrix[i];
	}
	Matrix4 mvp = proj * view;

	_gbuffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_terrain.draw(mvp.m);
	_grass.draw(mvp.m, _ctx, _blueNoise.texture());
	_trees.draw(mvp.m);

	_gbuffer.unbind();
}

void RenderGraph::skyPass()
{
	_sky.render(_lighting.litScene(), _gbuffer, _skyModel, _ctx);
}
void RenderGraph::deferredLightingPass()
{
	_lighting.render(_gbuffer, _shadow.depthTexture(), _ctx);
}
void RenderGraph::cloudPass()
{
	_cloud.render(_gbuffer, _ctx, _blueNoise.texture());
}
void RenderGraph::godrayPass()
{
	_godray.render(_gbuffer, _shadow.depthTexture(), _ctx,
				   _blueNoise.texture());
}
void RenderGraph::depthDownsamplePass()
{
	_depthDownsample.render(_gbuffer);
}
void RenderGraph::compositePass()
{
	_composite.render(_lighting.litScene(), _cloud.cloudRGBA(),
					  _godray.godrayRGB(), _gbuffer.depth(),
					  _depthDownsample.minDepth(), _skyModel, _ctx);
}
void RenderGraph::postFXPass()
{
	_postFX.render(_composite.hdrComposite(), _ctx);
}
