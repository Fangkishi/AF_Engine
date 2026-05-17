#pragma once

// GLVertexArray —— OpenGL 顶点数组对象实现
//
// 管理多组顶点缓冲区 + 索引缓冲区，自动配置顶点属性指针。
// 支持 float 和 int 类型的顶点属性。

#include "RHI/RHIVertexArray.h"

namespace AF {
namespace RHI {

class GLVertexArray : public RHIVertexArray
{
public:
    GLVertexArray();
    ~GLVertexArray() override;

    void Bind() const override;
    void Unbind() const override;

    void AddVertexBuffer(const Ref<RHIVertexBuffer>& vertexBuffer) override;
    void SetIndexBuffer(const Ref<RHIIndexBuffer>& indexBuffer) override;

    const std::vector<Ref<RHIVertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
    const Ref<RHIIndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
    uint32_t GetRendererID() const override { return m_RendererID; }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_VertexBufferIndex = 0;
    std::vector<Ref<RHIVertexBuffer>> m_VertexBuffers;
    Ref<RHIIndexBuffer> m_IndexBuffer;
};

} // namespace RHI
} // namespace AF
