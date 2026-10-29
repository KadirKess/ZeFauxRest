#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <string>
#include <vector>

class Shader
{
private:
	GLuint _programID;

	static std::string readFile(const char* path);
	static std::string injectDefines(const std::string& src,
									 const std::vector<std::string>& defines);
	static GLuint compileStage(GLenum type, const char* source,
							   const char* path);

public:
	Shader(const char* vertPath, const char* fragPath,
		   const std::vector<std::string>& defines = {});
	Shader(const char* computePath);
	Shader(const char* computePath, const std::vector<std::string>& defines);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	void setUniform1i(const char* name, int value) const;
	void setUniform1f(const char* name, float value) const;
	void setUniformVec2(const char* name, const float* vec) const;
	void setUniformVec3(const char* name, const float* vec) const;
	void setUniformMat4(const char* name, const float* mat) const;

	void use() const;

	GLuint id() const
	{
		return _programID;
	}
};
