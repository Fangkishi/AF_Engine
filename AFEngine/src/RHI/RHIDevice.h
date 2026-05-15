#pragma once

#include "Core/Types.h"
#include "RHI/RHIVertexArray.h"

#include <glm/glm.hpp>

namespace AF {
namespace RHI {

class RHIDevice : public NonCopyable
{
public:
    virtual ~RHIDevice() = default;

    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;

    virtual void DrawIndexed(const Ref<RHIVertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

    static Unique<RHIDevice> Create();
};

} // namespace RHI
} // namespace AF
