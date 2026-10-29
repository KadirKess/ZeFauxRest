#include "render/CloudNoise.hh"

#include <glad/glad.h>

CloudNoise::CloudNoise()
	: _shape(kShapeSize, kShapeSize, kShapeSize, GL_RGBA8, GL_LINEAR, GL_REPEAT,
			 "cloudShapeNoise")
	, _detail(kDetailSize, kDetailSize, kDetailSize, GL_R8, GL_LINEAR,
			  GL_REPEAT, "cloudDetailNoise")
{
	{
		Shader shapeShader(SHADER_DIR "/passes/cloud_noise_shape.comp");
		_shape.bindAsImage(0, GL_WRITE_ONLY);
		shapeShader.use();
		GLuint groups = (kShapeSize + 3) / 4;
		glDispatchCompute(groups, groups, groups);
	}
	{
		Shader detailShader(SHADER_DIR "/passes/cloud_noise_detail.comp");
		_detail.bindAsImage(0, GL_WRITE_ONLY);
		detailShader.use();
		GLuint groups = (kDetailSize + 3) / 4;
		glDispatchCompute(groups, groups, groups);
	}
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
					| GL_TEXTURE_FETCH_BARRIER_BIT);
}
