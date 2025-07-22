#pragma once
#include <cstdint>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <cassert>
#include <unordered_map>
#include "Texture.hpp"
#include <filesystem>
#include "tinyBVH.hpp"
#include "tiny_obj_loader.h"
#include "Common.hpp"

struct int2
{
	int x;
	int y;
};

struct uint2
{
	uint32_t x;
	uint32_t y;
};

static inline int cross(int2& a, int2& b, int2& p)
{
	int2 ab = { b.x - a.x, b.y - a.y };
	int2 ap = { p.x - a.x, p.y - a.y };
	return ab.x * ap.y - ab.y * ap.x;
}

struct int3
{
	int x;
	int y;
	int z;
};

struct float2
{
	float x;
	float y;

};

inline bool operator==(const float2& a, const float2& b)
{
	return a.x == b.x && a.y == b.y;
}

inline float2 operator-(const float2& a, const float2& b)
{
	return { a.x - b.x, a.y - b.y };
}

inline float2 operator+(const float2& a, const float2& b)
{
	return { a.x + b.x, a.y + b.y };
}

inline float2 operator*(const float2& a, const float& b)
{
	return { a.x * b, a.y * b };
}

struct float3
{
	float x;
	float y;
	float z;
};

inline float3 operator-(const float3 A, const float3& B)
{
	return
	{
		A.x - B.x, A.y - B.y, A.z - B.z
	};
};

inline float3 operator+(const float3 A, const float3& B)
{
	return
	{
		A.x + B.x, A.y + B.y, A.z + B.z
	};
};

inline float3 operator/(const float3 A, const float3& B)
{
	return
	{
		A.x / B.x, A.y / B.y, A.z / B.z
	};
};

inline float3 operator*(const float3 A, const float3& B)
{
	return
	{
		A.x* B.x, A.y* B.y, A.z* B.z
	};
};

inline float3 operator/(const float3 A, const float& B)
{
	return
	{
		A.x / B, A.y / B, A.z / B
	};
};

inline float3 operator*(const float3 A, const float& B)
{
	return
	{
		A.x * B, A.y * B, A.z * B
	};
};

static inline float Dot(float3 a, float3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float3 Normalize(float3 v)
{
	float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return { v.x / len, v.y / len, v.z / len };
}

static inline float3 normalize(float3 a)
{
	float len = sqrtf(Dot(a, a));
	if (len == 0.0f) return { 0, 0, 0 }; // avoid divide by zero
	return a / len;
}

static inline float3 Cross(float3 a, float3 b)
{
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

inline bool operator==(const float3& a, const float3& b) 
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct float4
{
	float x;
	float y;
	float z;
	float w;
};

struct mat4
{
	float m[4][4];
	static mat4 Identity()
	{
		mat4 mat{};
		mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.f;
		return mat;
	}
};

inline float4 operator*(const float4 V, const mat4& M)
{
	return {
		V.x * M.m[0][0] + V.y * M.m[1][0] + V.z * M.m[2][0] + V.w * M.m[3][0],
		V.x * M.m[0][1] + V.y * M.m[1][1] + V.z * M.m[2][1] + V.w * M.m[3][1],
		V.x * M.m[0][2] + V.y * M.m[1][2] + V.z * M.m[2][2] + V.w * M.m[3][2],
		V.x * M.m[0][3] + V.y * M.m[1][3] + V.z * M.m[2][3] + V.w * M.m[3][3]
	};
}

inline mat4 operator+(const mat4& A, const mat4& B)
{
	return {
		A.m[0][0] + B.m[0][0], A.m[1][0] + B.m[1][0], A.m[2][0] + B.m[2][0], A.m[3][0] + B.m[3][0],
		A.m[0][1] + B.m[0][1], A.m[1][1] + B.m[1][1], A.m[2][1] + B.m[2][1], A.m[3][1] + B.m[3][1],
		A.m[0][2] + B.m[0][2], A.m[1][2] + B.m[1][2], A.m[2][2] + B.m[2][2], A.m[3][2] + B.m[3][2],
		A.m[0][3] + B.m[0][3], A.m[1][3] + B.m[1][3], A.m[2][3] + B.m[2][3], A.m[3][3] + B.m[3][3]
	};
}

inline mat4 operator*(const mat4& A, const mat4& B)
{
	mat4 R{};
	for (int r = 0; r < 4; ++r)
		for (int c = 0; c < 4; ++c)
			for (int k = 0; k < 4; ++k)
				R.m[c][r] += A.m[k][r] * B.m[c][k];
	return R;
};

struct Vertex
{
	float3 position;
	float2 uv;
	float3 normal;
	float4 pos4() const { return { position.x, position.y, position.z, 1.f }; };

	bool operator==(const Vertex& other) const 
	{
		return (position == other.position &&
			uv == other.uv &&
			normal == other.normal);
	}

	float invW;     // = 1 / c.w
	float2 uvDivW;  // = uv * invW
};

struct Triangle
{
	uint32_t color;
	int indices[3];

	int materialIndex = -1; // default to invalid
};

struct Tri
{ 
	float3 vertex0, vertex1, vertex2; 
	float3 centroid; 
};

struct FaceVertex
{
	int vertexIndex = -1;
	int texCoordIndex = -1;
	int normalIndex = -1;
};

struct FaceGroup
{
	std::vector<FaceVertex> faceVertices;
	std::string name;
	int materialIndex = -1;  // Default to -1 = no material
};

class Mesh
{
public:
	Mesh() = default;
	Mesh(std::vector<Triangle> trianglesArg, std::vector<tinybvh::bvhvec4> fatTris, std::vector<Vertex> vertArg, std::vector<float2>texArg, std::vector<float3>normalArg, std::deque<FaceGroup> facesArg)
		: fatTriangles(fatTris), triangle(trianglesArg), vertices(vertArg), texCoords(texArg), normals(normalArg), face_groups(facesArg) { };
	std::vector<Vertex> vertices;
	std::vector<float2> texCoords;
	std::vector<float3> normals;
	std::vector<Triangle> triangle;
	std::vector<tinybvh::bvhvec4> fatTriangles;
	std::deque<FaceGroup> face_groups;

	std::vector<Texture>textures;
	int materialCount = 0;

};

static Mesh LoadMeshTinyObj(const char* filePath)
{
	Mesh mesh;
	std::string warn, err;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string base_dir = std::filesystem::path(filePath).parent_path().string();
	if (!base_dir.empty() && base_dir.back() != '/') base_dir += "/";

	bool ret = tinyobj::LoadObj(
		&attrib, &shapes, &materials, &warn, &err,
		filePath, base_dir.c_str(), true
	);

	if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
	if (!err.empty()) std::cerr << "ERR: " << err << std::endl;
	if (!ret) throw std::runtime_error("Failed to load OBJ");

	// Convert materials -> your Texture list
	for (const auto& mat : materials) {
		if (!mat.diffuse_texname.empty()) {
			std::string texPath = base_dir + mat.diffuse_texname;
			Texture tex(texPath, texPath);
			tex.name = mat.name;
			mesh.textures.push_back(tex);
			mesh.materialCount++;
		}
	}

	// Build a flat list of vertices, normals, texcoords
	std::unordered_map<std::string, int> uniqueVertexMap;
	std::vector<Vertex> finalVertices;
	std::vector<Triangle> finalTriangles;
	std::vector<tinybvh::bvhvec4> fatTris;

	for (const auto& shape : shapes) {
		size_t index_offset = 0;

		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
			int fv = shape.mesh.num_face_vertices[f];
			assert(fv == 3); // Only triangles due to triangulate=true

			Triangle tri;
			tri.materialIndex = shape.mesh.material_ids[f];

			for (int v = 0; v < 3; ++v) {
				tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
				Vertex vert{};

				// Position
				vert.position = {
					attrib.vertices[3 * idx.vertex_index + 0],
					attrib.vertices[3 * idx.vertex_index + 1],
					attrib.vertices[3 * idx.vertex_index + 2]
				};

				// Normal
				if (idx.normal_index >= 0) {
					vert.normal = {
						attrib.normals[3 * idx.normal_index + 0],
						attrib.normals[3 * idx.normal_index + 1],
						attrib.normals[3 * idx.normal_index + 2]
					};
				}

				// Texcoord
				if (idx.texcoord_index >= 0) {
					vert.uv = {
						attrib.texcoords[2 * idx.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] // Flip Y
					};
				}

				// Reuse vertex if already exists (optional)
				int finalIndex = static_cast<int>(finalVertices.size());
				finalVertices.push_back(vert);
				fatTris.push_back(tinybvh::bvhvec4{ vert.position.x, vert.position.y, vert.position.z, 0.f });

				tri.indices[v] = finalIndex;
			}

			finalTriangles.push_back(tri);
			index_offset += fv;
		}
	}

	mesh.triangle = finalTriangles;
	mesh.vertices = finalVertices;
	mesh.fatTriangles = fatTris;

	return mesh;
}

static inline void ParseFaceVertex(const std::string& tuple, FaceVertex& faceVert)
{
	std::istringstream stream(tuple);
	std::string part;

	std::getline(stream, part, '/');
	assert(!part.empty()); // ?? why not check here as well
	faceVert.vertexIndex = std::stoi(part) - 1;

	if (std::getline(stream, part, '/') && !part.empty()) 
	{
		faceVert.texCoordIndex = std::stoi(part) - 1;
	}

	if (std::getline(stream, part, '/') && !part.empty()) 
	{
		faceVert.normalIndex = std::stoi(part) - 1;
	}
}

static inline void ProcessFace(const std::vector<std::string>& tuple, std::vector<FaceVertex>& face_vertices)
{
	assert(tuple.size() == 3);
	for (const auto& t : tuple) 
	{
		FaceVertex face_vertex;
		ParseFaceVertex(t, face_vertex);
		face_vertices.push_back(face_vertex);
	}
}

static inline void ReadMTL(const char* filePath, Mesh* meshArg)
{
	std::ifstream file(filePath);
	std::string line;
	std::string currentMat;

	while (std::getline(file, line))
	{
		std::istringstream stream(line);
		std::string type;

		stream >> type;

		if (type == "newmtl")
		{
			stream >> currentMat;
		}
		else if (type == "map_Kd")
		{
			std::string texturePath;
			stream >> texturePath;

			if (!texturePath.empty())
			{
				std::string fullTexPath = std::filesystem::path(filePath).parent_path().string() + "/" + texturePath;

				Texture tex(fullTexPath, fullTexPath);
				tex.name = currentMat;

				meshArg->textures.push_back(tex);
				meshArg->materialCount++;
			}
		}
	}

	file.close();
}

template<typename T>
static inline T Clamp(const T& value, const T& minVal, const T& maxVal)
{
	if (value < minVal) return minVal;
	if (value > maxVal) return maxVal;
	return value;
}

static inline Mesh LoadMesh(const char* filePath)
{
	std::vector<Vertex> vertices;
	std::vector<float2> texCoords;
	std::vector<float3> normals;
	std::deque<FaceGroup> face_groups;
	std::vector<Triangle> triangles;
	std::vector<tinybvh::bvhvec4> fatTris;

	face_groups.emplace_back();
	FaceGroup* cur_face_group = &face_groups.back();

	std::ifstream file(filePath);
	std::string line;

	Mesh mesh;

	std::unordered_map<std::string, int> materialNameToIndex;
	int currentMaterialIndex = -1;

	std::string objDir = std::filesystem::path(filePath).parent_path().string();

	while(std::getline(file, line))
	{
		std::istringstream stream(line);
		std::string type;
		stream >> type;

		if (type == "mtllib")
		{
			std::string mtlFilename;
			stream >> mtlFilename;
			std::string fullPath = objDir + "/" + mtlFilename;
			ReadMTL(fullPath.c_str(), &mesh);

			// Create material name to index map
			for (int i = 0; i < mesh.textures.size(); ++i)
			{
				materialNameToIndex[mesh.textures[i].name] = i; // Assuming Texture has `name` field
			}
		}
		else if (type == "usemtl")
		{
			std::string matName;
			stream >> matName;

			auto it = materialNameToIndex.find(matName);
			if (it != materialNameToIndex.end())
			{
				currentMaterialIndex = it->second;
			}
			else
			{
				currentMaterialIndex = 99;
			}

			// If already faces in current group, start a new group
			if (!cur_face_group->faceVertices.empty())
			{
				face_groups.emplace_back();
				cur_face_group = &face_groups.back();
			}

			cur_face_group->materialIndex = currentMaterialIndex;
		}
		else if (type == "v") // VERTEX
		{
			float3 vertex;
			stream >> vertex.x >> vertex.y >> vertex.z;
			vertices.push_back(Vertex{ vertex });
			//fatTris.push_back(tinybvh::bvhvec4{ vertex.x, vertex.y, vertex.z, 0.f });
		}
		else if (type == "vt") // VERTEX TEXTURE COORD
		{
			float2 vt;
			stream >> vt.x >> vt.y;
			texCoords.push_back(vt);
		}
		else if (type == "vn") // VERTEX NORMAL
		{
			float3 vn;
			stream >> vn.x >> vn.y;
			normals.push_back(vn);
		}
		else if (type == "f") // FACE
		{
			std::vector<std::string> face;
			std::string tuple;
			while (stream >> tuple)
			{
				face.push_back(tuple);
			}
			ProcessFace(face, cur_face_group->faceVertices);
		}
		else if (type == "g") // GROUP
		{
			if (cur_face_group->faceVertices.size() != 0) 
			{
				face_groups.emplace_back();
				cur_face_group = &face_groups.back();
			}
			stream >> cur_face_group->name;
		}
	}

	file.close();

	for (const auto& group : face_groups) 
	{
		for (size_t n = 0; n < group.faceVertices.size(); n += 3) 
		{
			Triangle t;
			t.indices[0] = group.faceVertices[n].vertexIndex;
			t.indices[2] = group.faceVertices[n + 1].vertexIndex;
			t.indices[1] = group.faceVertices[n + 2].vertexIndex;
			t.materialIndex = group.materialIndex;
			triangles.push_back(t);
			
		}
	}

	std::vector<Triangle> finalTriangles;
	std::vector<Vertex> finalVertices;
	for (const auto& group : face_groups) 
	{
		for (size_t i = 0; i < group.faceVertices.size(); i += 3) 
		{
			Triangle t;

			for (int j = 0; j < 3; j++) 
			{
				const auto& fv = group.faceVertices[i + j];

				Vertex v;
				v.position = vertices[fv.vertexIndex].position;
				v.uv = texCoords[fv.texCoordIndex];
				v.normal = normals[fv.normalIndex];

				int index = static_cast<int>(finalVertices.size());
				fatTris.push_back(tinybvh::bvhvec4{ v.position.x, v.position.y, v.position.z, 0.f });

				finalVertices.push_back(v);
				t.indices[j] = index;
			}
			t.materialIndex = group.materialIndex;
			finalTriangles.push_back(t);
		}
	}

	mesh.triangle = finalTriangles;
	mesh.fatTriangles = fatTris;
	mesh.vertices = finalVertices;
	mesh.texCoords = texCoords;
	mesh.normals = normals;
	mesh.face_groups = face_groups;

	return mesh;

	//return Mesh(finalTriangles, fatTris, finalVertices, texCoords, normals, face_groups);
};

namespace mat
{
	inline mat4 Transpose(const mat4& m)
	{
		mat4 t;

		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				t.m[i][j] = m.m[j][i];

		return t;
	}

	static inline mat4 Translate(const float pX, const float pY, const float pZ)
	{
		return {
			1, 0, 0, pX,
			0, 1, 0, pY,
			0, 0, 1, pZ,
			0, 0, 0, 1
		};
	}

	static inline mat4 Rotate(const float rX, const float rY, const float rZ, const float theta)
	{
		float len = sqrt(rX * rX + rY * rY + rZ * rZ);
		if (len == 0.0f) return mat4::Identity(); // or handle as error

		float x = rX / len;
		float y = rY / len;
		float z = rZ / len;

		float c = cos(theta);
		float s = sin(theta);
		float t = 1.f - c;

		return {
			t * x * x + c,     t * x * y - s * z,  t * x * z + s * y,  0.f,
			t * x * y + s * z, t * y * y + c,      t * y * z - s * x,  0.f,
			t * x * z - s * y, t * y * z + s * x,  t * z * z + c,      0.f,
			0.f,               0.f,                0.f,                1.f
		};
	}

	static inline mat4 Scale(const float sX, const float sY, const float sZ)
	{
		return {
			sX, 0, 0, 0,
			0, sY, 0, 0,
			0, 0, sZ, 0,
			0, 0, 0, 1
		};
	}

	// TODO: These are blackboxed now learn about them
	static inline mat4 LookAt(float3 eye, float3 target, float3 up)
	{
		float3 zaxis = Normalize(eye - target);
		float3 xaxis = Normalize(Cross(up, zaxis));
		float3 yaxis = Cross(zaxis, xaxis);

		mat4 view = mat4::Identity();

		view.m[0][0] = xaxis.x;  view.m[0][1] = xaxis.y;  view.m[0][2] = xaxis.z;
		view.m[1][0] = yaxis.x;  view.m[1][1] = yaxis.y;  view.m[1][2] = yaxis.z;
		view.m[2][0] = zaxis.x;  view.m[2][1] = zaxis.y;  view.m[2][2] = zaxis.z;

		view.m[3][0] = -Dot(xaxis, eye);
		view.m[3][1] = -Dot(yaxis, eye);
		view.m[3][2] = -Dot(zaxis, eye);

		return view;
	}

	// TODO: These are blackboxed now learn about them
	static inline mat4 Perspective(float fovY, float aspect, float zn, float zf)
	{
		float yScale = 1.0f / tanf(fovY / 2.0f);
		float xScale = yScale / aspect;

		return mat4
		{
			xScale, 0,      0,                          0,
			0,      yScale, 0,                          0,
			0,      0,      zf / (zf - zn),             1,
			0,      0,      -zn * zf / (zf - zn),       0
		};
	};
};

static inline float3 operator*(const mat4& mat, const float3& vec) 
{
	float x = mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z + mat.m[0][3];
	float y = mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z + mat.m[1][3];
	float z = mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z + mat.m[2][3];
	float w = mat.m[3][0] * vec.x + mat.m[3][1] * vec.y + mat.m[3][2] * vec.z + mat.m[3][3];

	// Perspective divide, in case the matrix includes projection
	if (w != 0.0f && w != 1.0f) 
	{
		x /= w;
		y /= w;
		z /= w;
	}

	return float3(x, y, z);
}

static inline uint32_t MakeColor(float R, float G, float B, float A)
{
	return (static_cast<uint32_t>(B * 255) << 0 |
		static_cast<uint32_t>(G * 255) << 8 |
		static_cast<uint32_t>(R * 255) << 16 |
		static_cast<uint32_t>(A * 255) << 24);
}

// Integer version: expects R, G, B, A in [0, 255] range
static inline uint32_t MakeColor(int R, int G, int B, int A)
{
	return (static_cast<uint32_t>(B) << 0 |
		static_cast<uint32_t>(G) << 8 |
		static_cast<uint32_t>(R) << 16 |
		static_cast<uint32_t>(A) << 24);
}

static inline uint32_t Lehmer32()
{
	uint32_t nLehmer = 0;
	nLehmer += 0xe120fc15;
	uint64_t tmp;
	tmp = (uint64_t)nLehmer * 0x4a39b70d;
	uint32_t m1 = (tmp >> 32) ^ tmp;
	tmp = (uint64_t)m1 * 0x12fad5c9;
	uint32_t m2 = (tmp >> 32) ^ tmp;
	return m2;
}

static inline double rndDouble(double min, double max) // Random double in this range
{
	return ((double)Lehmer32() / (double)(0x7FFFFFFF)) * (max - min) + min;
}

static inline int rndInt(int min, int max) // Random int in this range
{
	return (Lehmer32() % (max - min)) + min;
}

static inline mat4 InverseAffine(const mat4& m)
{
	float3 a = { m.m[0][0], m.m[0][1], m.m[0][2] }; // Right
	float3 b = { m.m[1][0], m.m[1][1], m.m[1][2] }; // Up
	float3 c = { m.m[2][0], m.m[2][1], m.m[2][2] }; // Forward
	float3 t = { m.m[3][0], m.m[3][1], m.m[3][2] }; // Translation

	// Invert rotation part (transpose)
	float3 iA = { a.x, b.x, c.x };
	float3 iB = { a.y, b.y, c.y };
	float3 iC = { a.z, b.z, c.z };

	// Invert translation
	float3 iT = {
		-Dot(iA, t),
		-Dot(iB, t),
		-Dot(iC, t)
	};

	mat4 inv = mat4::Identity();
	inv.m[0][0] = iA.x; inv.m[0][1] = iA.y; inv.m[0][2] = iA.z;
	inv.m[1][0] = iB.x; inv.m[1][1] = iB.y; inv.m[1][2] = iB.z;
	inv.m[2][0] = iC.x; inv.m[2][1] = iC.y; inv.m[2][2] = iC.z;
	inv.m[3][0] = iT.x; inv.m[3][1] = iT.y; inv.m[3][2] = iT.z;

	return inv;
}