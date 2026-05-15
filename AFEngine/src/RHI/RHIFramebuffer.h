#pragma once

#include "Core/Types.h"
#include "RHI/RHITexture.h"

namespace AF {
namespace RHI {

class RHIFramebuffer : public NonCopyable
{
public:
    virtual ~RHIFramebuffer() = default;

    virtual void AttachColor(const Ref<RHITexture2D>& texture, uint32_t slot = 0) = 0;
    virtual void AttachDepth(const Ref<RHITexture2D>& texture) = 0;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetColorAttachmentID(uint32_t slot = 0) const = 0;

    static Unique<RHIFramebuffer> Create();
};

} // namespace RHI
} // namespace AF
