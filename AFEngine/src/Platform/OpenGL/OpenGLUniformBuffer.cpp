#include "afpch.h"
#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

namespace AF {

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		m_Binding = binding;
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		if (data == nullptr && size == 0) {
			// 清空整个SSBO缓冲区
			uint32_t zero = 0;
			glClearNamedBufferData(m_RendererID, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
			return;
		}

		if (data == nullptr && size > 0) {
			// 如果data为nullptr但size不为0，用零填充指定区域
			std::vector<uint8_t> zeroData(size, 0);
			glNamedBufferSubData(m_RendererID, offset, size, zeroData.data());
			return;
		}

		// 正常情况
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLUniformBuffer::Bind()
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_RendererID);
	}

	////////////////////////////////////
	////////////////SSBO////////////////
	////////////////////////////////////

	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(uint32_t size, uint32_t binding)
	{
		m_Binding = binding;
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLShaderStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		if (data == nullptr && size == 0) {
			// 清空整个SSBO缓冲区
			uint32_t zero = 0;
			glClearNamedBufferData(m_RendererID, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
			return;
		}

		if (data == nullptr && size > 0) {
			// 如果data为nullptr但size不为0，用零填充指定区域
			std::vector<uint8_t> zeroData(size, 0);
			glNamedBufferSubData(m_RendererID, offset, size, zeroData.data());
			return;
		}

		// 正常情况
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLShaderStorageBuffer::Bind()
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Binding, m_RendererID);
	}

}
