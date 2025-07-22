#include "Model.hpp"

Model::Model(const char* filePath)
{
	mesh = LoadMeshTinyObj(filePath);

	modelBVH = new tinybvh::BVH8_CPU();
	modelBVH->BuildHQ(mesh.fatTriangles.data(), static_cast<uint32_t>((mesh.fatTriangles.size() / 3)));
}