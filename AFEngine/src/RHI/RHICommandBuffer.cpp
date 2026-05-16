#include "RHI/RHICommandBuffer.h"
#include "RHI/RHIDevice.h"

#include <glad/glad.h>

namespace AF {
namespace RHI {

template<class... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};

    template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

static GLenum ToGLDepthFunc(DepthCompareFunc func)
{
    switch (func)
    {
        case DepthCompareFunc::Never:          return GL_NEVER;
        case DepthCompareFunc::Less:           return GL_LESS;
        case DepthCompareFunc::Equal:          return GL_EQUAL;
        case DepthCompareFunc::LessEqual:      return GL_LEQUAL;
        case DepthCompareFunc::Greater:        return GL_GREATER;
        case DepthCompareFunc::NotEqual:       return GL_NOTEQUAL;
        case DepthCompareFunc::GreaterEqual:   return GL_GEQUAL;
        case DepthCompareFunc::Always:         return GL_ALWAYS;
    }
    return GL_LESS;
}

static GLenum ToGLCullFace(CullMode cull)
{
    switch (cull)
    {
        case CullMode::None:         return GL_NONE;
        case CullMode::Front:        return GL_FRONT;
        case CullMode::Back:         return GL_BACK;
        case CullMode::FrontAndBack: return GL_FRONT_AND_BACK;
    }
    return GL_BACK;
}

static GLenum ToGLFillMode(FillMode fill)
{
    switch (fill)
    {
        case FillMode::Solid:      return GL_FILL;
        case FillMode::Wireframe:  return GL_LINE;
        case FillMode::Point:      return GL_POINT;
    }
    return GL_FILL;
}

void RHICommandBuffer::Begin()
{
    m_Commands.clear();
    m_Recording = true;
}

void RHICommandBuffer::End()
{
    m_Recording = false;
}

void RHICommandBuffer::Reset()
{
    m_Commands.clear();
}

void RHICommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    m_Commands.push_back(CmdSetViewport{ x, y, w, h });
}

void RHICommandBuffer::SetClearColor(const glm::vec4& color)
{
    m_Commands.push_back(CmdSetClearColor{ color });
}

void RHICommandBuffer::Clear()
{
    m_Commands.push_back(CmdClear{});
}

void RHICommandBuffer::BindShader(const Ref<RHIShader>& shader)
{
    m_Commands.push_back(CmdBindShader{ shader });
}

void RHICommandBuffer::SetMat4(const std::string& name, const glm::mat4& value)
{
    m_Commands.push_back(CmdSetMat4{ name, value });
}

void RHICommandBuffer::SetFloat4(const std::string& name, const glm::vec4& value)
{
    m_Commands.push_back(CmdSetFloat4{ name, value });
}

void RHICommandBuffer::SetFloat3(const std::string& name, const glm::vec3& value)
{
    m_Commands.push_back(CmdSetFloat3{ name, value });
}

void RHICommandBuffer::SetFloat(const std::string& name, float value)
{
    m_Commands.push_back(CmdSetFloat{ name, value });
}

void RHICommandBuffer::SetInt(const std::string& name, int value)
{
    m_Commands.push_back(CmdSetInt{ name, value });
}

void RHICommandBuffer::BindTexture(uint32_t slot, const Ref<RHITexture2D>& texture)
{
    m_Commands.push_back(CmdBindTexture{ slot, texture });
}

void RHICommandBuffer::DrawIndexed(const Ref<RHIVertexArray>& vao, uint32_t indexCount)
{
    m_Commands.push_back(CmdDrawIndexed{ vao, indexCount });
}

void RHICommandBuffer::BindFramebuffer(RHIFramebuffer* fbo)
{
    m_Commands.push_back(CmdBindFramebuffer{ fbo });
}

void RHICommandBuffer::UnbindFramebuffer(RHIFramebuffer* fbo)
{
    m_Commands.push_back(CmdUnbindFramebuffer{ fbo });
}

void RHICommandBuffer::BindUniformBuffer(RHIUniformBuffer* buffer, uint32_t binding)
{
    m_Commands.push_back(CmdBindUniformBuffer{ buffer, binding });
}

void RHICommandBuffer::SetBufferData(RHIUniformBuffer* buffer, const void* data, uint32_t size, uint32_t offset)
{
    auto bytes = static_cast<const uint8_t*>(data);
    m_Commands.push_back(CmdSetBufferData{ buffer, offset, std::vector<uint8_t>(bytes, bytes + size) });
}

void RHICommandBuffer::BindStorageBuffer(RHIStorageBuffer* buffer, uint32_t binding)
{
    m_Commands.push_back(CmdBindStorageBuffer{ buffer, binding });
}

void RHICommandBuffer::SetStorageBufferData(RHIStorageBuffer* buffer, const void* data, uint32_t size, uint32_t offset)
{
    auto bytes = static_cast<const uint8_t*>(data);
    m_Commands.push_back(CmdSetStorageBufferData{ buffer, offset, std::vector<uint8_t>(bytes, bytes + size) });
}

void RHICommandBuffer::SetDepthStencilState(bool depthTest, bool depthWrite, DepthCompareFunc depthFunc)
{
    m_Commands.push_back(CmdSetDepthStencilState{ depthFunc, depthTest, depthWrite });
}

void RHICommandBuffer::SetRasterizerState(CullMode cull, FrontFace winding, FillMode fill)
{
    m_Commands.push_back(CmdSetRasterizerState{ cull, winding, fill });
}

void RHICommandBuffer::SetBlendState(uint32_t attachment, bool enable)
{
    m_Commands.push_back(CmdSetBlendState{ attachment, enable });
}

void RHICommandBuffer::Execute(RHIDevice& device)
{
    Ref<RHIShader> currentShader;

    for (auto& cmd : m_Commands)
    {
        std::visit(Overloaded{
            [&](const CmdSetViewport& c)
            {
                device.SetViewport(c.X, c.Y, c.Width, c.Height);
            },
            [&](const CmdSetClearColor& c)
            {
                device.SetClearColor(c.Color);
            },
            [&](const CmdClear&)
            {
                device.Clear();
            },
            [&](const CmdBindShader& c)
            {
                c.Shader->Bind();
                currentShader = c.Shader;
            },
            [&](const CmdSetMat4& c)
            {
                if (currentShader) currentShader->SetMat4(c.Name, c.Value);
            },
            [&](const CmdSetFloat4& c)
            {
                if (currentShader) currentShader->SetFloat4(c.Name, c.Value);
            },
            [&](const CmdSetFloat3& c)
            {
                if (currentShader) currentShader->SetFloat3(c.Name, c.Value);
            },
            [&](const CmdSetFloat& c)
            {
                if (currentShader) currentShader->SetFloat(c.Name, c.Value);
            },
            [&](const CmdSetInt& c)
            {
                if (currentShader) currentShader->SetInt(c.Name, c.Value);
            },
            [&](const CmdBindTexture& c)
            {
                c.Texture->Bind(c.Slot);
            },
            [&](const CmdDrawIndexed& c)
            {
                device.DrawIndexed(c.VAO, c.IndexCount);
            },
            [&](const CmdBindFramebuffer& c)
            {
                c.FBO->Bind();
            },
            [&](const CmdUnbindFramebuffer& c)
            {
                c.FBO->Unbind();
            },
            [&](const CmdBindUniformBuffer& c)
            {
                c.Buffer->Bind(c.Binding);
            },
            [&](const CmdSetBufferData& c)
            {
                c.Buffer->SetData(c.Data.data(), static_cast<uint32_t>(c.Data.size()), c.Offset);
            },
            [&](const CmdBindStorageBuffer& c)
            {
                c.Buffer->Bind(c.Binding);
            },
            [&](const CmdSetStorageBufferData& c)
            {
                c.Buffer->SetData(c.Data.data(), static_cast<uint32_t>(c.Data.size()), c.Offset);
            },
            [&](const CmdSetDepthStencilState& c)
            {
                if (c.DepthTest) glEnable(GL_DEPTH_TEST);
                else             glDisable(GL_DEPTH_TEST);
                glDepthMask(c.DepthWrite ? GL_TRUE : GL_FALSE);
                glDepthFunc(ToGLDepthFunc(c.DepthFunc));
            },
            [&](const CmdSetRasterizerState& c)
            {
                if (c.Cull == CullMode::None)
                    glDisable(GL_CULL_FACE);
                else
                {
                    glEnable(GL_CULL_FACE);
                    glCullFace(ToGLCullFace(c.Cull));
                }
                glFrontFace(c.Winding == FrontFace::CW ? GL_CW : GL_CCW);
                glPolygonMode(GL_FRONT_AND_BACK, ToGLFillMode(c.Fill));
            },
            [&](const CmdSetBlendState& c)
            {
                if (c.Enable)
                {
                    glEnablei(GL_BLEND, c.Attachment);
                    glBlendFuncSeparatei(c.Attachment,
                        c.SrcColor == BlendFactor::One ? GL_ONE : GL_SRC_ALPHA,
                        c.DstColor == BlendFactor::Zero ? GL_ZERO : GL_ONE_MINUS_SRC_ALPHA,
                        c.SrcAlpha == BlendFactor::One ? GL_ONE : GL_ONE,
                        c.DstAlpha == BlendFactor::Zero ? GL_ZERO : GL_ONE_MINUS_SRC_ALPHA);
                }
                else
                {
                    glDisablei(GL_BLEND, c.Attachment);
                }
            },
        }, cmd);
    }
}

} // namespace RHI
} // namespace AF
