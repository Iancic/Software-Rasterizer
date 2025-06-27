#pragma once
#include "stb_image.h"
#include <string>

class Texture
{
public:
	Texture() = default;
	Texture(const std::string filePath);
	~Texture() = default;

	const uint8_t GetTexel(int index);

	const int GetWidth() { return width; }
	const int GetHeight() { return height; }
	const int GetChannels() { return nrChannels; };
	const unsigned char* GetBuffer() { buffer; };

private:
	int width, height, nrChannels;
	unsigned char* buffer;
};