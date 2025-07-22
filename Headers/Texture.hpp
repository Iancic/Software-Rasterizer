#pragma once
#include "stb_image.h"
#include <string>
#include <cstdint>

class Texture
{
public:
    std::string name; // Material name (not filepath)
    int width = 0, height = 0, nrChannels = 0;
    uint8_t* buffer = nullptr;

    Texture(const std::string& filePath, const std::string& materialName);
    ~Texture();

    uint8_t GetTexel(int x, int y, int channel = 0) const;

    bool IsValid() const { return buffer != nullptr; }

    int GetWidth() { return width; };
    int GetHeight() { return height; };
};