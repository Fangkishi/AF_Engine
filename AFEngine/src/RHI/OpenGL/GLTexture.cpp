#include "RHI/OpenGL/GLTexture.h"

#include <glad/glad.h>
#include <stb_image.h>

#include "Core/Assert.h"
#include "Core/Log.h"

namespace AF {
namespace RHI {

static void GLFormatFromTextureFormat(TextureFormat format, uint32_t& internalFormat, uint32_t& dataFormat)
{
    switch (format)
    {
        case TextureFormat::RGBA8:          internalFormat = GL_RGBA8;   dataFormat = GL_RGBA;  return;
        case TextureFormat::RGB8:           internalFormat = GL_RGB8;    dataFormat = GL_RGB;   return;
        case TextureFormat::RGBA16F:        internalFormat = GL_RGBA16F; dataFormat = GL_RGBA;  return;
        case TextureFormat::R8:             internalFormat = GL_R8;      dataFormat = GL_RED;   return;
        case TextureFormat::Depth32:        internalFormat = GL_DEPTH_COMPONENT32F; dataFormat = GL_DEPTH_COMPONENT; return;
        case TextureFormat::Depth24Stencil8: internalFormat = GL_DEPTH24_STENCIL8; dataFormat = GL_DEPTH_STENCIL;    return;
        default:                            internalFormat = GL_RGBA8;  dataFormat = GL_RGBA;  return;
    }
}

GLTexture2D::GLTexture2D(uint32_t width, uint32_t height, TextureFormat format)
{
    CreateBlank(width, height, format);
}

GLTexture2D::GLTexture2D(const std::string& path)
{
    LoadFromFile(path);
}

GLTexture2D::~GLTexture2D()
{
    glDeleteTextures(1, &m_RendererID);
}

void GLTexture2D::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_RendererID);
}

void GLTexture2D::CreateBlank(uint32_t width, uint32_t height, TextureFormat format)
{
    m_Width = width;
    m_Height = height;
    m_Format = format;
    GLFormatFromTextureFormat(format, m_InternalFormat, m_DataFormat);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

    GLenum filter = (format == TextureFormat::Depth32 || format == TextureFormat::Depth24Stencil8)
        ? GL_NEAREST : GL_LINEAR;

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GLTexture2D::LoadFromFile(const std::string& path)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data)
    {
        AF_LOG_ERROR("Failed to load texture: {}", path);
        AF_DEBUGBREAK();
        return;
    }

    m_Width = static_cast<uint32_t>(width);
    m_Height = static_cast<uint32_t>(height);

    switch (channels)
    {
        case 1: m_Format = TextureFormat::R8;    break;
        case 3: m_Format = TextureFormat::RGB8;  break;
        case 4: default: m_Format = TextureFormat::RGBA8; break;
    }
    GLFormatFromTextureFormat(m_Format, m_InternalFormat, m_DataFormat);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(m_RendererID);

    stbi_image_free(data);

    AF_LOG_INFO("Loaded texture: {} ({}x{})", path, m_Width, m_Height);
}

Ref<RHITexture2D> RHITexture2D::Create(uint32_t width, uint32_t height, TextureFormat format)
{
    return std::make_shared<GLTexture2D>(width, height, format);
}

Ref<RHITexture2D> RHITexture2D::Create(const std::string& path)
{
    return std::make_shared<GLTexture2D>(path);
}

} // namespace RHI
} // namespace AF
