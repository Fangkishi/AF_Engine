#include "RHI/OpenGL/GLStorageBuffer.h"

#include <glad/glad.h>

#include "Core/Log.h"

namespace AF {
namespace RHI {

GLStorageBuffer::GLStorageBuffer(uint32_t size, uint32_t binding)
    : m_Binding(binding)
    , m_Size(size)
{
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferStorage(m_RendererID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    AF_LOG_INFO("GLStorageBuffer: created id={} size={} binding={}", m_RendererID, size, binding);
}

GLStorageBuffer::~GLStorageBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void GLStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    glNamedBufferSubData(m_RendererID, offset, size, data);
}

void GLStorageBuffer::Bind(uint32_t binding)
{
    m_Binding = binding;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
}

Ref<RHIStorageBuffer> RHIStorageBuffer::Create(uint32_t size, uint32_t binding)
{
    return std::make_shared<GLStorageBuffer>(size, binding);
}

} // namespace RHI
} // namespace AF
