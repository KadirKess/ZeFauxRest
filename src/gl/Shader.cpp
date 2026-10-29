#include "gl/Shader.hh"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

static std::string resolveIncludes(const std::string& src,
								   const std::string& dir)
{
	std::istringstream in(src);
	std::ostringstream out;
	std::string line;
	while (std::getline(in, line))
	{
		if (line.rfind("#include", 0) == 0)
		{
			auto q1 = line.find('"');
			auto q2 = line.rfind('"');
			if (q1 == std::string::npos || q1 == q2)
				throw std::runtime_error("Malformed #include: " + line);
			std::string rel = line.substr(q1 + 1, q2 - q1 - 1);
			std::string incPath = dir + "/" + rel;
			std::ifstream inc(incPath);
			if (!inc.is_open())
				throw std::runtime_error("Cannot open include: " + incPath);
			std::ostringstream buf;
			buf << inc.rdbuf();
			out << resolveIncludes(buf.str(), dir) << "\n";
		}
		else
		{
			out << line << "\n";
		}
	}
	return out.str();
}

static std::string dirOf(const char* path)
{
	std::string s(path);
	auto slash = s.rfind('/');
	return slash == std::string::npos ? "." : s.substr(0, slash);
}

std::string Shader::injectDefines(const std::string& src,
								  const std::vector<std::string>& defines)
{
	if (defines.empty())
		return src;

	// #version must stay the first token, so the defines slot in right after
	// it.
	std::istringstream in(src);
	std::ostringstream out;
	std::string line;
	bool injected = false;
	while (std::getline(in, line))
	{
		out << line << "\n";
		if (!injected && line.rfind("#version", 0) == 0)
		{
			for (const auto& def : defines)
				out << "#define " << def << "\n";
			injected = true;
		}
	}
	return out.str();
}

std::string Shader::readFile(const char* path)
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error(std::string("Failed to open shader file: ")
								 + path);
	std::stringstream ss;
	ss << file.rdbuf();
	return resolveIncludes(ss.str(), dirOf(path));
}

GLuint Shader::compileStage(GLenum type, const char* source, const char* path)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		throw std::runtime_error(
			std::string("Shader compilation failed (file: ") + path + ") "
			+ infoLog);
	}
	return shader;
}

static void checkLink(GLuint program)
{
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		throw std::runtime_error(std::string("Shader program linking failed: ")
								 + infoLog);
	}
}

Shader::Shader(const char* vertPath, const char* fragPath,
			   const std::vector<std::string>& defines)
{
	std::string vertCode = injectDefines(readFile(vertPath), defines);
	std::string fragCode = injectDefines(readFile(fragPath), defines);

	GLuint vertShader =
		compileStage(GL_VERTEX_SHADER, vertCode.c_str(), vertPath);
	GLuint fragShader =
		compileStage(GL_FRAGMENT_SHADER, fragCode.c_str(), fragPath);

	_programID = glCreateProgram();
	glAttachShader(_programID, vertShader);
	glAttachShader(_programID, fragShader);
	glLinkProgram(_programID);
	checkLink(_programID);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

Shader::Shader(const char* computePath)
	: Shader(computePath, std::vector<std::string>{})
{}

Shader::Shader(const char* computePath, const std::vector<std::string>& defines)
{
	std::string code = injectDefines(readFile(computePath), defines);
	GLuint compShader =
		compileStage(GL_COMPUTE_SHADER, code.c_str(), computePath);

	_programID = glCreateProgram();
	glAttachShader(_programID, compShader);
	glLinkProgram(_programID);
	checkLink(_programID);

	glDeleteShader(compShader);
}

Shader::~Shader()
{
	glDeleteProgram(_programID);
}

void Shader::setUniform1i(const char* name, int value) const
{
	glUniform1i(glGetUniformLocation(_programID, name), value);
}

void Shader::setUniform1f(const char* name, float value) const
{
	glUniform1f(glGetUniformLocation(_programID, name), value);
}

void Shader::setUniformVec2(const char* name, const float* vec) const
{
	glUniform2fv(glGetUniformLocation(_programID, name), 1, vec);
}

void Shader::setUniformVec3(const char* name, const float* vec) const
{
	glUniform3fv(glGetUniformLocation(_programID, name), 1, vec);
}

void Shader::setUniformMat4(const char* name, const float* mat) const
{
	// Matrix4 is row-major so GL_TRUE tells the driver to transpose on upload
	glUniformMatrix4fv(glGetUniformLocation(_programID, name), 1, GL_TRUE, mat);
}

void Shader::use() const
{
	glUseProgram(_programID);
}
