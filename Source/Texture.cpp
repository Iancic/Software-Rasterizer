#include "Texture.hpp"

Texture::Texture(const std::string& filePath, const std::string& materialName)
{
    name = materialName;

    //stbi_set_flip_vertically_on_load(true); // optional but helps most renderers

    buffer = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
    if (!buffer) {
    }
}

Texture::~Texture()
{
}

uint8_t Texture::GetTexel(int x, int y, int channel) const
{
    if (!buffer || x < 0 || y < 0 || x >= width || y >= height || channel >= nrChannels)
        return 0;

    int index = (y * width + x) * nrChannels + channel;
    return buffer[index];
}