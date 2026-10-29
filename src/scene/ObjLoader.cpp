#define TINYOBJLOADER_IMPLEMENTATION
#include "scene/ObjLoader.hh"

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "tiny_obj_loader.h"

ObjMesh loadObj(const char* path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	// tinyobj resolves the mtllib relative to this base dir, not to the .obj
	// path - without it the materials (and so the colours) never load.
	std::string dir(path);
	size_t slash = dir.find_last_of("/\\");
	dir =
		(slash == std::string::npos) ? std::string() : dir.substr(0, slash + 1);

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path,
						  dir.c_str()))
		throw std::runtime_error(std::string("OBJ load failed: ") + err);

	ObjMesh mesh;

	struct Key
	{
		int vi, ni, ti;
		bool operator==(const Key& o) const
		{
			return vi == o.vi && ni == o.ni && ti == o.ti;
		}
	};
	struct KeyHash
	{
		size_t operator()(const Key& k) const
		{
			size_t h = (size_t)k.vi * 73856093;
			h ^= (size_t)k.ni * 19349663;
			h ^= (size_t)k.ti * 83492791;
			return h;
		}
	};
	std::unordered_map<Key, unsigned int, KeyHash> seen;

	for (const auto& shape : shapes)
	{
		ObjGroup group;
		group.name = shape.name;
		group.indexOffset = (unsigned int)mesh.indices.size();

		for (size_t i = 0; i < shape.mesh.indices.size(); ++i)
		{
			const auto& idx = shape.mesh.indices[i];

			// One material per face (each face is a triangle). Its colour is
			// copied onto the vertices below because the tree meshes only carry named
			// materials, no texture coords or per-vertex colours.
			float mr = 1.0f, mg = 1.0f, mb = 1.0f;
			bool haveMat = false;
			size_t faceIdx = i / 3;
			if (faceIdx < shape.mesh.material_ids.size())
			{
				int matId = shape.mesh.material_ids[faceIdx];
				if (matId >= 0 && matId < (int)materials.size())
				{
					mr = materials[matId].diffuse[0];
					mg = materials[matId].diffuse[1];
					mb = materials[matId].diffuse[2];
					haveMat = true;
				}
			}

			Key key{ idx.vertex_index, idx.normal_index, idx.texcoord_index };
			auto it = seen.find(key);
			if (it != seen.end())
			{
				mesh.indices.push_back(it->second);
			}
			else
			{
				unsigned int newIdx = (unsigned int)(mesh.vertices.size()
													 / ObjMesh::kVertexStride);
				seen[key] = newIdx;
				mesh.indices.push_back(newIdx);

				float px = attrib.vertices[3 * idx.vertex_index + 0];
				float py = attrib.vertices[3 * idx.vertex_index + 1];
				float pz = attrib.vertices[3 * idx.vertex_index + 2];

				float nx = 0.0f, ny = 1.0f, nz = 0.0f;
				if (idx.normal_index >= 0)
				{
					nx = attrib.normals[3 * idx.normal_index + 0];
					ny = attrib.normals[3 * idx.normal_index + 1];
					nz = attrib.normals[3 * idx.normal_index + 2];
				}

				float tu = 0.0f, tv = 0.0f;
				if (idx.texcoord_index >= 0)
				{
					tu = attrib.texcoords[2 * idx.texcoord_index + 0];
					tv = attrib.texcoords[2 * idx.texcoord_index + 1];
				}

				// Pick the colour, use the face's material if it has one, else
				// the per-vertex colour. The loader fills in white when a file
				// has no vertex colours, so the material wins when it's there.
				float cr = 1.0f, cg = 1.0f, cb = 1.0f;
				if (haveMat)
				{
					cr = mr;
					cg = mg;
					cb = mb;
				}
				else if (!attrib.colors.empty())
				{
					cr = attrib.colors[3 * idx.vertex_index + 0];
					cg = attrib.colors[3 * idx.vertex_index + 1];
					cb = attrib.colors[3 * idx.vertex_index + 2];
				}

				mesh.vertices.insert(
					mesh.vertices.end(),
					{ px, py, pz, nx, ny, nz, tu, tv, cr, cg, cb });
			}
		}

		group.indexCount =
			(unsigned int)mesh.indices.size() - group.indexOffset;
		mesh.groups.push_back(std::move(group));
	}

	return mesh;
}
