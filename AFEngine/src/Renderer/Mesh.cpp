#include "Renderer/Mesh.h"

namespace AF {

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const RHI::BufferLayout& layout)
{
    m_IndexCount = static_cast<uint32_t>(indices.size());

    // 创建顶点缓冲区并设置布局
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

} // namespace AF
