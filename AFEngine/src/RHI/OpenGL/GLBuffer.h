#pragma once

#include "RHI/RHIBuffer.h"

namespace AF {
namespace RHI {

class GLVertexBuffer : public RHIVertexBuffer
{
public:
    GLVertexBuffer(uint32_t size);
    GLVertexBuffer(float* vertices, uint32_t size);
    ~GLVertexBuffer() override;

    void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
    const BufferLayout& GetLayout() const override { return m_Layout; }

    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID = 0;
    BufferLayout m_Layout;
};

class GLIndexBuffer : public RHIIndexBuffer
{
public:
    GLIndexBuffer(uint32_t* indices, uint32_t count);
    ~GLIndexBuffer() override;

    uint32_t GetCount() const override { return m_Count; }
    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_Count = 0;
};

} // namespace RHI
} // namespace AF
