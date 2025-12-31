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
			case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH_STENCIL;
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
			case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
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

		GLenum target = m_Specification.ArraySize > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
		glCreateTextures(target, 1, &m_RendererID);
		if (m_Specification.ArraySize > 1)
		{
			glTextureStorage3D(m_RendererID, 1, m_InternalFormat,
				m_Width, m_Height, m_Specification.ArraySize);
		}
		else
		{
			glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		}

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
		uint32_t layerSize = m_Width * m_Height * bpp;

		if (m_Specification.ArraySize > 1)
		{
			// 纹理数组：验证总大小是否匹配所有层
			AF_CORE_ASSERT(size == layerSize * m_Specification.ArraySize,
				"Data size mismatch for texture array!");

			// 写入整个数组
			glTextureSubImage3D(m_RendererID, 0, 0, 0, 0,
				m_Width, m_Height, m_Specification.ArraySize,
				m_DataFormat, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			// 单纹理处理
			AF_CORE_ASSERT(size == layerSize, "Data must be entire texture!");
			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height,
				m_DataFormat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		AF_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}

	OpenGLTextureCube::OpenGLTextureCube(const TextureSpecification& specification)
		: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
	{
		AF_PROFILE_FUNCTION();

		m_InternalFormat = Utils::AFImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = Utils::AFImageFormatToGLDataFormat(m_Specification.Format);

		GLenum target = m_Specification.ArraySize > 1 ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
		glCreateTextures(target, 1, &m_RendererID);
		if (m_Specification.ArraySize > 1)
		{
			glTextureStorage3D(m_RendererID, 1, m_InternalFormat,
				m_Width, m_Height, 6 * m_Specification.ArraySize);
		}
		else
		{
			glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		}

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	OpenGLTextureCube::OpenGLTextureCube(const std::string& path, const bool isSRGB)
		: m_Path(path)
	{
		AF_PROFILE_FUNCTION();
		LoadFromFile(path, isSRGB);
	}

	OpenGLTextureCube::OpenGLTextureCube(uint32_t rendererID, uint32_t width, uint32_t height)
		: m_RendererID(rendererID), m_Width(width), m_Height(height)
	{
		// 直接使用已有的纹理ID，不创建新纹理
		m_IsExternal = true; // 标记为外部纹理，析构时不删除
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		AF_PROFILE_FUNCTION();
		if (!m_IsExternal) {
			glDeleteTextures(1, &m_RendererID);
		}
	}

	void OpenGLTextureCube::LoadFromFile(const std::string& path, bool isSRGB)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(0); // 立方体贴图通常不需要垂直翻转

		stbi_uc* data = nullptr;
		{
			//AF_PROFILE_SCOPE("stbi_load - OpenGLTextureCube::LoadFromFile");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}

		if (data)
		{
			m_IsLoaded = true;
			m_Width = width / 4; // 假设是十字形布局的立方体贴图
			m_Height = height / 3;

			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				if (isSRGB)
				{
					internalFormat = GL_SRGB8_ALPHA8;
				}
				else
				{
					internalFormat = GL_RGBA8;
				}
				dataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				if (isSRGB)
				{
					internalFormat = GL_SRGB8;
				}
				else
				{
					internalFormat = GL_RGB8;
				}
				dataFormat = GL_RGB;
			}

			m_InternalFormat = internalFormat;
			m_DataFormat = dataFormat;

			AF_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

			// 设置纹理参数
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			// 注意：这里需要根据实际的立方体贴图布局来分割图像数据
			// 这是一个简化版本，实际使用时需要根据具体的文件格式来处理
			// 这里假设数据已经是正确的立方体贴图格式

			stbi_image_free(data);
		}
		else
		{
			AF_CORE_ERROR("Failed to load cubemap texture: {0}", path);
		}
	}

	void OpenGLTextureCube::SetData(void* data, uint32_t size)
	{
		AF_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		uint32_t faceSize = m_Width * m_Height * bpp;
		uint32_t totalFaces = 6 * m_Specification.ArraySize;

		AF_CORE_ASSERT(size == faceSize * totalFaces, "Data must match cube map array size!");

		GLenum target = m_Specification.ArraySize > 1 ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;

		if (m_Specification.ArraySize > 1)
		{
			// 写入整个立方体贴图数组
			glTextureSubImage3D(m_RendererID, 0, 0, 0, 0,
				m_Width, m_Height, totalFaces,
				m_DataFormat, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			// 单立方体贴图处理
			GLenum faces[] = {
				GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
			};

			for (int i = 0; i < 6; i++)
			{
				void* faceData = static_cast<uint8_t*>(data) + faceSize * i;
				glTextureSubImage3D(m_RendererID, 0, 0, 0, i, m_Width, m_Height, 1,
					m_DataFormat, GL_UNSIGNED_BYTE, faceData);
			}
		}
	}

	void OpenGLTextureCube::Bind(uint32_t slot) const
	{
		AF_PROFILE_FUNCTION();
		glBindTextureUnit(slot, m_RendererID);
	}
}
