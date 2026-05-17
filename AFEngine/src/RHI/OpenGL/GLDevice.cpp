#include "RHI/OpenGL/GLDevice.h"

#include <glad/glad.h>

#include "Core/Log.h"
#include "RHI/OpenGL/GLVertexArray.h"

namespace AF {
namespace RHI {

GLDevice::GLDevice()
{
    AF_LOG_INFO("GLDevice initialized");
}

GLDevice::~GLDevice()
{
    AF_LOG_INFO("GLDevice shutdown");
}

void GLDevice::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(static_cast<GLint>(x), static_cast<GLint>(y),
               static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void GLDevice::SetClearColor(const glm::vec4& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void GLDevice::Clear()
{
    // 默认清除颜色缓冲 + 深度缓冲
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLDevice::DrawIndexed(const Ref<RHIVertexArray>& vertexArray, uint32_t indexCount)
{
    vertexArray->Bind();
    auto& ib = vertexArray->GetIndexBuffer();
    uint32_t count = indexCount > 0 ? indexCount : ib->GetCount();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

// 平台工厂
Unique<RHIDevice> RHIDevice::Create()
{
    return std::make_unique<GLDevice>();
}

} // namespace RHI
} // namespace AF
