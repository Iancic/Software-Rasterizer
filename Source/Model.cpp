#include "Model.hpp"

Model::Model()
{
	texture = new Texture("Assets/container.jpg");
	mesh = LoadMesh("Assets/Teapot/Teapot.obj");

	modelBVH = new tinybvh::BVH8_CPU();
	modelBVH->BuildHQ(mesh.fatTriangles.data(), static_cast<uint32_t>((mesh.fatTriangles.size() / 3)));
}