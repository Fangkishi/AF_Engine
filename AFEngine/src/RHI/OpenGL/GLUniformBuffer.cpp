#include "RHI/OpenGL/GLUniformBuffer.h"

#include <glad/glad.h>

#include "Core/Log.h"

namespace AF {
namespace RHI {

GLUniformBuffer::GLUniformBuffer(uint32_t size, uint32_t binding)
    : m_Binding(binding)
    , m_Size(size)
{
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferStorage(m_RendererID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    AF_LOG_INFO("GLUniformBuffer: created id={} size={} binding={}", m_RendererID, size, binding);
}

GLUniformBuffer::~GLUniformBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void GLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    glNamedBufferSubData(m_RendererID, offset, size, data);
}

void GLUniformBuffer::Bind(uint32_t binding)
{
    m_Binding = binding;
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
}

Ref<RHIUniformBuffer> RHIUniformBuffer::Create(uint32_t size, uint32_t binding)
{
    return std::make_shared<GLUniformBuffer>(size, binding);
}

} // namespace RHI
} // namespace AF
