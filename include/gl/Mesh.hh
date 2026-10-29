#pragma once

#include <glad/glad.h>
#include <vector>

class Mesh
{
private:
	GLuint _vao;
	GLuint _vbo;
	GLuint _ebo;
	GLsizei _indexCount;

public:
	Mesh(const std::vector<float>& vertices,
		 const std::vector<unsigned int>& indices);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&&) noexcept;
	Mesh& operator=(Mesh&&) noexcept;

	void draw() const;

	GLuint vao() const
	{
		return _vao;
	}
};
