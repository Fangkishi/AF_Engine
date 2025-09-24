#include "afpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>

namespace AF {

	namespace Utils {
		static GLenum AFImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8: return GL_RGB;
			case ImageFormat::RGBA8: return GL_RGBA;
			case ImageFormat::SRGB8: return GL_SRGB8;
			case ImageFormat::SRGBA8: return GL_SRGB8_ALPHA8;
			}

			//AF_CORE_ASSERT(false);
			return 0;
		}

		static GLenum AFImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8: return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
			case ImageFormat::SRGB8: return GL_SRGB8;
			case ImageFormat::SRGBA8: return GL_SRGB8_ALPHA8;
			}

			//AF_CORE_ASSERT(false);
			return 0;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
		: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
	{
		AF_PROFILE_FUNCTION();

		m_InternalFormat = Utils::AFImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = Utils::AFImageFormatToGLDataFormat(m_Specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, const bool isSRGB)
		: m_Path(path)
	{
		AF_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			//AF_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}

		if (data)
		{
			m_IsLoaded = true;

			m_Width = width;
			m_Height = height;

			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				if (isSRGB)
				{
					internalFormat = GL_SRGB8_ALPHA8;  // sRGB空间
				}
				else
				{
					internalFormat = GL_RGBA8;         // 线性空间（法线、金属度等）
				}
				dataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				if (isSRGB)
				{
					internalFormat = GL_SRGB8;         // sRGB空间
				}
				else
				{
					internalFormat = GL_RGB8;          // 线性空间
				}
				dataFormat = GL_RGB;
			}

			m_InternalFormat = internalFormat;
			m_DataFormat = dataFormat;

			AF_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t rendererID, uint32_t width, uint32_t height)
		: m_RendererID(rendererID), m_Width(width), m_Height(height)
	{
		// 直接使用现有的纹理 ID，不创建新纹理
		m_IsExternal = true; // 标记为外部纹理，析构时不删除
	}


	OpenGLTexture2D::~OpenGLTexture2D()
	{
		AF_PROFILE_FUNCTION();
		if (!m_IsExternal) {
			glDeleteTextures(1, &m_RendererID);
		}
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		AF_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		AF_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		AF_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}
}
