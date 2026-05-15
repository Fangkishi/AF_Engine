#include "RHI/OpenGL/GLVertexArray.h"
#include "RHI/OpenGL/GLBuffer.h"

#include <glad/glad.h>

namespace AF {
namespace RHI {

GLVertexArray::GLVertexArray()
{
    glCreateVertexArrays(1, &m_RendererID);
}

GLVertexArray::~GLVertexArray()
{
    glDeleteVertexArrays(1, &m_RendererID);
}

void GLVertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
}

void GLVertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void GLVertexArray::AddVertexBuffer(const Ref<RHIVertexBuffer>& vertexBuffer)
{
    glBindVertexArray(m_RendererID);

    auto* glVB = static_cast<GLVertexBuffer*>(vertexBuffer.get());
    glBindBuffer(GL_ARRAY_BUFFER, glVB->GetRendererID());

    const auto& layout = vertexBuffer->GetLayout();
    for (const auto& element : layout)
    {
        GLenum glType;
        switch (element.Type)
        {
            case ShaderDataType::Float:  glType = GL_FLOAT;         break;
            case ShaderDataType::Float2: glType = GL_FLOAT;         break;
            case ShaderDataType::Float3: glType = GL_FLOAT;         break;
            case ShaderDataType::Float4: glType = GL_FLOAT;         break;
            case ShaderDataType::Mat3:   glType = GL_FLOAT;         break;
            case ShaderDataType::Mat4:   glType = GL_FLOAT;         break;
            case ShaderDataType::Int:    glType = GL_INT;           break;
            case ShaderDataType::Int2:   glType = GL_INT;           break;
            case ShaderDataType::Int3:   glType = GL_INT;           break;
            case ShaderDataType::Int4:   glType = GL_INT;           break;
            case ShaderDataType::Bool:   glType = GL_BOOL;          break;
            default: glType = GL_FLOAT; break;
        }

        glEnableVertexAttribArray(m_VertexBufferIndex);
        if (glType == GL_INT || glType == GL_BOOL)
        {
            glVertexAttribIPointer(m_VertexBufferIndex, element.GetComponentCount(), glType,
                                   layout.GetStride(), reinterpret_cast<const void*>(static_cast<uintptr_t>(element.Offset)));
        }
        else
        {
            glVertexAttribPointer(m_VertexBufferIndex, element.GetComponentCount(), glType,
                                  element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                                  reinterpret_cast<const void*>(static_cast<uintptr_t>(element.Offset)));
        }
        ++m_VertexBufferIndex;
    }

    m_VertexBuffers.push_back(vertexBuffer);
}

void GLVertexArray::SetIndexBuffer(const Ref<RHIIndexBuffer>& indexBuffer)
{
    glBindVertexArray(m_RendererID);
    auto* glIB = static_cast<GLIndexBuffer*>(indexBuffer.get());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIB->GetRendererID());
    m_IndexBuffer = indexBuffer;
}

// Factory
Ref<RHIVertexArray> RHIVertexArray::Create()
{
    return std::make_shared<GLVertexArray>();
}

} // namespace RHI
} // namespace AF
