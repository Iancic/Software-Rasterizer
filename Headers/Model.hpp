#pragma once
#include "tinyBVH.hpp"
#include <vector>
#include "Math.hpp"

class Model
{
public:
	Model(const char* filePath);
	~Model() = default;

	Mesh mesh;

	std::vector<float4> fatTriangles; // Fat Triangles For Tinybvh

	// BVH
	tinybvh::BVH8_CPU* modelBVH;

private:
};