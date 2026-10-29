#pragma once

#include "gl/Shader.hh"
#include "gl/Texture.hh"

class GBuffer;

// min() of each 2x2 gDepth quad so thin foliage survives the bilateral upsample
// R32F color rather than depth attachment because compute can't imageLoad depth
class DepthDownsamplePass
{
private:
	Texture _minDepth;
	Shader _shader;

public:
	DepthDownsamplePass(int width, int height);

	DepthDownsamplePass(const DepthDownsamplePass&) = delete;
	DepthDownsamplePass& operator=(const DepthDownsamplePass&) = delete;

	void render(const GBuffer& gbuffer);

	const Texture& minDepth() const
	{
		return _minDepth;
	}
};
