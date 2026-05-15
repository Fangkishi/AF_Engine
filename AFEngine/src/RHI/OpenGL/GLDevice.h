#pragma once

#include "RHI/RHIDevice.h"

namespace AF {
namespace RHI {

class GLDevice : public RHIDevice
{
public:
    GLDevice();
    ~GLDevice() override;

    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void SetClearColor(const glm::vec4& color) override;
    void Clear() override;

    void DrawIndexed(const Ref<RHIVertexArray>& vertexArray, uint32_t indexCount = 0) override;
};

} // namespace RHI
} // namespace AF
