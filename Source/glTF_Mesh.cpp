#include "glTF_Mesh.hpp"

glTF_Mesh::glTF_Mesh(const std::string& path)
{
    std::string err;
    std::string warn;

    bool ret = m_loader.LoadASCIIFromFile(&m_model, &err, &warn, path);

    if (!ret) 
    {
        std::cout << "Failed to parse glTF" << std::endl;
        if (!warn.empty())std::cout << "Warn: " << warn << std::endl;
        if (!err.empty()) std::cout << "Err: " << err << std::endl;
    }
    else
    {
        std::cout << "Loaded glTF file successfully, meshes: " << m_model.meshes.size() << std::endl;
        ExtractMesh();
    }
}

void glTF_Mesh::ExtractMesh()
{
    // Access meshes 
    for (size_t i = 0; i < m_model.meshes.size(); ++i) 
    {
        // and their primitives
        const auto& mesh = m_model.meshes[i];
        std::cout << "Mesh " << i << ": " << mesh.name << std::endl;

        for (size_t j = 0; j < mesh.primitives.size(); ++j) 
        {
            const auto& primitive = mesh.primitives[j];

            // A. Populate Triangle vector
            ExtractTriangles(primitive);

            // B. Populate Contiguous vectors
            // Position
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) 
            {
                int posAccessor = primitive.attributes.at("POSITION");
                m_positions = GetAccessorData<glm::vec3>(m_model, posAccessor);
                std::cout << "Vertex count: " << m_positions.size() << std::endl;
            }

            // Normals
            if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
            {
                int posAccessor = primitive.attributes.at("NORMAL");
                m_normals = GetAccessorData<glm::vec3>(m_model, posAccessor);
            }

            // Texture Coordinates 0 
            if (primitive.attributes.find("TEXCOORD_0 ") != primitive.attributes.end())
            {
                int posAccessor = primitive.attributes.at("TEXCOORD_0 ");
                m_texcoords_0 = GetAccessorData<glm::vec2>(m_model, posAccessor);
            }

            // Texture Coordinates 1 
            if (primitive.attributes.find("TEXCOORD_1 ") != primitive.attributes.end())
            {
                int posAccessor = primitive.attributes.at("TEXCOORD_1 ");
                m_texcoords_1 = GetAccessorData<glm::vec2>(m_model, posAccessor);
            }

            // C. Populate Vertex vector (only for current custom rasterizer)
            ExtractVertices(primitive);

        }
    }
}

void glTF_Mesh::ExtractTriangles(const tinygltf::Primitive& primitive)
{
    if (primitive.mode != TINYGLTF_MODE_TRIANGLES) std::cout << "Warning: Only triangle mode fully supported" << std::endl;

    // Indexed geometry
    if (primitive.indices >= 0) 
    {
        const auto& accessor = m_model.accessors[primitive.indices];

        auto indices = GetAccessorData<uint32_t>(m_model, primitive.indices);

        for (size_t i = 0; i < indices.size(); i += 3) 
            m_triangles.push_back(new Triangle(indices[i], indices[i + 1], indices[i + 2]));  
    }
    // Non-indexed geometry - vertices define triangles directly
    else 
    {
        if (primitive.attributes.find("POSITION") != primitive.attributes.end()) 
        {
            int posAccessor = primitive.attributes.at("POSITION");
            const auto& accessor = m_model.accessors[posAccessor];

            for (size_t i = 0; i < accessor.count; i += 3)
                m_triangles.push_back(new Triangle(i, i+1, i+2));
        }
    }

    std::cout << "Triangle count:" << m_triangles.size() << std::endl;
}

void glTF_Mesh::ExtractVertices(const tinygltf::Primitive& primitive)
{
    // I extract position to store inside vertices for the rasterizer algorithm
    // See: Game::RenderObject()

    int posAccessor = primitive.attributes.at("POSITION");
    const auto& accessor = m_model.accessors[posAccessor];
    size_t vertexCount = accessor.count;

    m_vertices.reserve(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i) 
    {
        Vertex* vertex = new Vertex();

        // Position required for the algorithm
        vertex->m_position = m_positions[i];

        m_vertices.push_back(vertex);
    }

    std::cout << "Vertices extracted: " << vertexCount << std::endl;
}

void glTF_Mesh::PopulateBuffers()
{
    // TODO: AGC Buffers
}