#pragma once

// RHIVertexArray —— 顶点数组对象抽象接口
//
// 封装多组顶点缓冲区 + 索引缓冲区 + 顶点属性布局。

#include "Core/Types.h"
#include "RHI/RHIBuffer.h"

#include <vector>

namespace AF {
namespace RHI {

class RHIVertexArray
{
public:
    virtual ~RHIVertexArray() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const Ref<RHIVertexBuffer>& vertexBuffer) = 0;
    virtual void SetIndexBuffer(const Ref<RHIIndexBuffer>& indexBuffer) = 0;

    virtual const std::vector<Ref<RHIVertexBuffer>>& GetVertexBuffers() const = 0;
    virtual const Ref<RHIIndexBuffer>& GetIndexBuffer() const = 0;

    virtual uint32_t GetRendererID() const = 0;

    static Ref<RHIVertexArray> Create();
};

} // namespace RHI
} // namespace AF
