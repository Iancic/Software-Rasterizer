#pragma once
#include "Texture.hpp"
#include "tinyBVH.hpp"
#include <vector>
#include "Math.hpp"

class Model
{
public:
	Model();
	~Model() = default;

	Mesh mesh;
	Texture* texture = nullptr;

	std::vector<float4> fatTriangles; // Fat Triangles For Tinybvh

	// BVH
	tinybvh::BVH8_CPU* modelBVH;

private:
};