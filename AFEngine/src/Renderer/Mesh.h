#pragma once

#include "Core/Types.h"
#include "RHI/RHIVertexArray.h"
#include "RHI/RHIBuffer.h"
#include "RHI/RHITypes.h"

#include <vector>
#include <cstdint>

namespace AF {

class Mesh
{
public:
    Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const RHI::BufferLayout& layout);
    ~Mesh() = default;

    const Ref<RHI::RHIVertexArray>& GetVertexArray() const { return m_VertexArray; }
    uint32_t GetIndexCount() const { return m_IndexCount; }

    static Ref<Mesh> CreateTriangle();
    static Ref<Mesh> CreateQuad(float size = 1.0f);

private:
    Ref<RHI::RHIVertexArray> m_VertexArray;
    uint32_t m_IndexCount = 0;
};

} // namespace AF
