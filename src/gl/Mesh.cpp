#include "gl/Mesh.hh"

Mesh::Mesh(const std::vector<float>& vertices,
		   const std::vector<unsigned int>& indices)
	: _indexCount(static_cast<GLsizei>(indices.size()))
{
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);

	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
				 vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
				 indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
						  (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
						  (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
						  (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &_vao);
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_ebo);
}

Mesh::Mesh(Mesh&& other) noexcept
	: _vao(other._vao)
	, _vbo(other._vbo)
	, _ebo(other._ebo)
	, _indexCount(other._indexCount)
{
	other._vao = 0;
	other._vbo = 0;
	other._ebo = 0;
	other._indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this != &other)
	{
		glDeleteVertexArrays(1, &_vao);
		glDeleteBuffers(1, &_vbo);
		glDeleteBuffers(1, &_ebo);

		_vao = other._vao;
		_vbo = other._vbo;
		_ebo = other._ebo;
		_indexCount = other._indexCount;

		other._vao = 0;
		other._vbo = 0;
		other._ebo = 0;
		other._indexCount = 0;
	}
	return *this;
}

void Mesh::draw() const
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
