#pragma once

#include <string>
#include <vector>

struct ObjGroup
{
	std::string name;
	unsigned int indexOffset;
	unsigned int indexCount;
};

// Color falls back to (1,1,1) when the OBJ has no `x y z r g b` extension.
struct ObjMesh
{
	static constexpr int kVertexStride = 11;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<ObjGroup> groups;
};

ObjMesh loadObj(const char* path);
