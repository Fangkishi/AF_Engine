#pragma once

// RHIBuffer —— 顶点和索引缓冲区抽象接口

#include "Core/Types.h"
#include "RHI/RHITypes.h"

namespace AF {
namespace RHI {

class RHIVertexBuffer
{
public:
    virtual ~RHIVertexBuffer() = default;

    virtual void SetLayout(const BufferLayout& layout) = 0;
    virtual const BufferLayout& GetLayout() const = 0;

    static Ref<RHIVertexBuffer> Create(uint32_t size);
    static Ref<RHIVertexBuffer> Create(float* vertices, uint32_t size);
};

class RHIIndexBuffer
{
public:
    virtual ~RHIIndexBuffer() = default;

    virtual uint32_t GetCount() const = 0;

    static Ref<RHIIndexBuffer> Create(uint32_t* indices, uint32_t count);
};

} // namespace RHI
} // namespace AF
