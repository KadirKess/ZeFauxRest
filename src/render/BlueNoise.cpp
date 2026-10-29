#include "render/BlueNoise.hh"

#include <glad/glad.h>

BlueNoise::BlueNoise()
	: _shader(SHADER_DIR "/passes/blue_noise.comp")
	, _texture(kSize, kSize, GL_R8, GL_NEAREST, GL_REPEAT, "blueNoise")
{
	_texture.bindAsImage(0, GL_WRITE_ONLY);
	_shader.use();

	GLuint groups = (kSize + 15) / 16;
	glDispatchCompute(groups, groups, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT);
}
