#include "Renderer/Mesh.h"

namespace AF {

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const RHI::BufferLayout& layout)
{
    m_IndexCount = static_cast<uint32_t>(indices.size());

    auto vb = RHI::RHIVertexBuffer::Create(
        const_cast<float*>(vertices.data()),
        static_cast<uint32_t>(vertices.size() * sizeof(float)));
    vb->SetLayout(layout);

    auto ib = RHI::RHIIndexBuffer::Create(
        const_cast<uint32_t*>(indices.data()),
        m_IndexCount);

    m_VertexArray = RHI::RHIVertexArray::Create();
    m_VertexArray->AddVertexBuffer(vb);
    m_VertexArray->SetIndexBuffer(ib);
}

Ref<Mesh> Mesh::CreateTriangle()
{
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,
    };

    std::vector<uint32_t> indices = { 0, 1, 2 };

    RHI::BufferLayout layout = {
        { RHI::ShaderDataType::Float3, "a_Position" },
        { RHI::ShaderDataType::Float3, "a_Color"    },
    };

    return std::make_shared<Mesh>(vertices, indices, layout);
}

Ref<Mesh> Mesh::CreateQuad(float size)
{
    float h = size * 0.5f;

    std::vector<float> vertices = {
        -h, -h, 0.0f,   0.0f, 0.0f,
         h, -h, 0.0f,   1.0f, 0.0f,
         h,  h, 0.0f,   1.0f, 1.0f,
        -h,  h, 0.0f,   0.0f, 1.0f,
    };

    std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

    RHI::BufferLayout layout = {
        { RHI::ShaderDataType::Float3, "a_Position" },
        { RHI::ShaderDataType::Float2, "a_TexCoord" },
    };

    return std::make_shared<Mesh>(vertices, indices, layout);
}

} // namespace AF
