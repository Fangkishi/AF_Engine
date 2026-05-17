#pragma once

// GLStorageBuffer —— OpenGL Shader Storage Buffer 实现
//
// 使用 DSA glCreateBuffers / glNamedBufferStorage 创建，
// 通过 glBindBufferBase(GL_SHADER_STORAGE_BUFFER) 绑定。
// 比 UBO 容量更大，适合传递光源列表等动态数据。

#include "RHI/RHIStorageBuffer.h"

namespace AF {
namespace RHI {

class GLStorageBuffer : public RHIStorageBuffer
{
public:
    GLStorageBuffer(uint32_t size, uint32_t binding);
    ~GLStorageBuffer() override;

    void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
    void Bind(uint32_t binding) override;

    uint32_t GetRendererID() const override { return m_RendererID; }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_Binding = 0;
    uint32_t m_Size = 0;
};

} // namespace RHI
} // namespace AF
