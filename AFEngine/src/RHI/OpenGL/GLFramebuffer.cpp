#include "RHI/OpenGL/GLFramebuffer.h"

#include <glad/glad.h>

#include "Core/Log.h"

namespace AF {
namespace RHI {

GLFramebuffer::GLFramebuffer()
{
    glCreateFramebuffers(1, &m_RendererID);
}

GLFramebuffer::~GLFramebuffer()
{
    glDeleteFramebuffers(1, &m_RendererID);
}

void GLFramebuffer::Invalidate()
{
    m_ColorAttachments.clear();
    m_DepthAttachment.reset();
    m_Width = 0;
    m_Height = 0;
}

void GLFramebuffer::AttachColor(const Ref<RHITexture2D>& texture, uint32_t slot)
{
    m_Width = texture->GetWidth();
    m_Height = texture->GetHeight();

    glNamedFramebufferTexture(m_RendererID, GL_COLOR_ATTACHMENT0 + slot, texture->GetRendererID(), 0);

    Attachment att;
    att.Texture = texture;
    att.Slot = slot;
    m_ColorAttachments.push_back(att);
}

void GLFramebuffer::AttachDepth(const Ref<RHITexture2D>& texture)
{
    m_Width = texture->GetWidth();
    m_Height = texture->GetHeight();

    glNamedFramebufferTexture(m_RendererID, GL_DEPTH_ATTACHMENT, texture->GetRendererID(), 0);
    m_DepthAttachment = texture;
}

void GLFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

    // 设置 MRT 绘制缓冲区
    std::vector<GLenum> drawBuffers;
    for (const auto& att : m_ColorAttachments)
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + att.Slot);
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    // 将视口设置为 FBO 尺寸
    glViewport(0, 0, m_Width, m_Height);
}

void GLFramebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t GLFramebuffer::GetColorAttachmentID(uint32_t slot) const
{
    for (const auto& att : m_ColorAttachments)
        if (att.Slot == slot)
            return att.Texture->GetRendererID();
    return 0;
}

// 工厂
Unique<RHIFramebuffer> RHIFramebuffer::Create()
{
    return std::make_unique<GLFramebuffer>();
}

} // namespace RHI
} // namespace AF
