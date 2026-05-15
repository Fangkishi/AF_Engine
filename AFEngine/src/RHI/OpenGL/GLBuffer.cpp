#include "RHI/OpenGL/GLBuffer.h"

#include <glad/glad.h>

namespace AF {
namespace RHI {

GLVertexBuffer::GLVertexBuffer(uint32_t size)
{
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
}

GLVertexBuffer::GLVertexBuffer(float* vertices, uint32_t size)
{
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferData(m_RendererID, size, vertices, GL_STATIC_DRAW);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

Ref<RHIVertexBuffer> RHIVertexBuffer::Create(uint32_t size)
{
    return std::make_shared<GLVertexBuffer>(size);
}

Ref<RHIVertexBuffer> RHIVertexBuffer::Create(float* vertices, uint32_t size)
{
    return std::make_shared<GLVertexBuffer>(vertices, size);
}

GLIndexBuffer::GLIndexBuffer(uint32_t* indices, uint32_t count)
    : m_Count(count)
{
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferData(m_RendererID, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

GLIndexBuffer::~GLIndexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

Ref<RHIIndexBuffer> RHIIndexBuffer::Create(uint32_t* indices, uint32_t count)
{
    return std::make_shared<GLIndexBuffer>(indices, count);
}

} // namespace RHI
} // namespace AF
