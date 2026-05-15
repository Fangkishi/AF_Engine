#pragma once

#include "RHI/RHIFramebuffer.h"

#include <vector>

namespace AF {
namespace RHI {

class GLFramebuffer : public RHIFramebuffer
{
public:
    GLFramebuffer();
    ~GLFramebuffer() override;

    void AttachColor(const Ref<RHITexture2D>& texture, uint32_t slot = 0) override;
    void AttachDepth(const Ref<RHITexture2D>& texture) override;

    void Bind() override;
    void Unbind() override;

    uint32_t GetWidth() const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }
    uint32_t GetColorAttachmentID(uint32_t slot = 0) const override;

private:
    void Invalidate();

    uint32_t m_RendererID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    struct Attachment
    {
        Ref<RHITexture2D> Texture;
        uint32_t Slot = 0;
    };
    std::vector<Attachment> m_ColorAttachments;
    Ref<RHITexture2D> m_DepthAttachment;
};

} // namespace RHI
} // namespace AF
