#pragma once

// Mesh —— 网格资源（顶点 + 索引缓冲区封装）
//
// 构造时直接创建 RHI 顶点/索引缓冲区并组装到 VertexArray。

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

private:
    Ref<RHI::RHIVertexArray> m_VertexArray;
    uint32_t m_IndexCount = 0;
};

} // namespace AF
