#include "Texture.hpp"

Texture::Texture(const std::string filePath)
{
	buffer = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
}

const uint8_t Texture::GetTexel(int index)
{
	return buffer[index];
}
