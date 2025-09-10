#include "Model.hpp"

Model::Model(const char* filePath)
{
	mesh = LoadMeshTinyObj(filePath);
}