#pragma once

#include "Core/Types.h"

#include <cstdint>

namespace AF {
namespace RHI {

class RHIUniformBuffer : public NonCopyable
{
public:
    virtual ~RHIUniformBuffer() = default;

    virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
    virtual void Bind(uint32_t binding) = 0;

    virtual uint32_t GetRendererID() const = 0;

    static Ref<RHIUniformBuffer> Create(uint32_t size, uint32_t binding = 0);
};

} // namespace RHI
} // namespace AF
