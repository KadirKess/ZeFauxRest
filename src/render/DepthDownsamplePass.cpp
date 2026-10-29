#include "render/DepthDownsamplePass.hh"

#include <glad/glad.h>

#include "render/GBuffer.hh"

DepthDownsamplePass::DepthDownsamplePass(int width, int height)
	// Nearest filtering because the bilateral upsample fetches exact depth candidates,
	// it never wants the hardware to interpolate depths across a silhouette.
	: _minDepth(width / 2, height / 2, GL_R32F, GL_NEAREST, GL_CLAMP_TO_EDGE,
				"minDepthHalf")
	, _shader(SHADER_DIR "/passes/depth_downsample.comp")
{
	glObjectLabel(GL_PROGRAM, _shader.id(), -1, "Depth downsample shader");
}

void DepthDownsamplePass::render(const GBuffer& gbuffer)
{
	gbuffer.depth().bindToUnit(0);
	_minDepth.bindAsImage(0, GL_WRITE_ONLY);

	_shader.use();
	_shader.setUniform1i("gDepth", 0);

	GLuint groupsX = (_minDepth.width() + 15) / 16;
	GLuint groupsY = (_minDepth.height() + 15) / 16;
	glDispatchCompute(groupsX, groupsY, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT);
}
