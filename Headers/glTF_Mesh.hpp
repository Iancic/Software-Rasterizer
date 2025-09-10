#pragma once
#include "tiny_gltf.h"
#include "Math.hpp"

class glTF_Mesh
{

public:
    struct Vertex
    {
        glm::vec3 m_position;
        glm::vec2 m_texcoord_0;
        glm::vec2 m_texcoord_1;
        glm::vec3 m_normals;

        glm::vec4 pos4() const { return { m_position.x, m_position.y, m_position.z, 1.f }; };
    };

    class Triangle
    {
    public:
        Triangle(uint32_t i0, uint32_t i1, uint32_t i2) : m_indices{ i0, i1, i2 } {}
        Triangle() : m_indices{ 0, 0, 0 } {}

        std::array<uint32_t, 3> m_indices;
    };

	glTF_Mesh(const std::string& path);
	~glTF_Mesh() = default;

	tinygltf::Model m_model;
	tinygltf::TinyGLTF m_loader;

    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec2> m_texcoords_0; // primary
    std::vector<glm::vec2> m_texcoords_1; // secondary
    std::vector<glm::vec3> m_normals;

    std::vector<glTF_Mesh::Vertex*> m_vertices;
    std::vector<glTF_Mesh::Triangle*> m_triangles;

    void ExtractMesh();
    void ExtractTriangles(const tinygltf::Primitive& primitive);
    void ExtractVertices(const tinygltf::Primitive& primitive);

    void PopulateBuffers(); // AGC Buffers

    // Generate by Claude AI:
    // Helper function to get typed data from an accessor
    template<typename T>
    std::vector<T> GetAccessorData(const tinygltf::Model& model, int accessorIndex) 
    {
        const auto& accessor = model.accessors[accessorIndex];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];

        const unsigned char* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

        std::vector<T> result(accessor.count);
        memcpy(result.data(), data, accessor.count * sizeof(T));

        return result;
    }



};