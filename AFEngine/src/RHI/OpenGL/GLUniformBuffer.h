#pragma once

#include "RHI/RHIUniformBuffer.h"

namespace AF {
namespace RHI {

class GLUniformBuffer : public RHIUniformBuffer
{
public:
    GLUniformBuffer(uint32_t size, uint32_t binding);
    ~GLUniformBuffer() override;

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
