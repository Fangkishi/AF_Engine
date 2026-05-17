#include "RHI/OpenGL/GLTexture.h"

#include <glad/glad.h>
#include <stb_image.h>

#include "Core/Assert.h"
#include "Core/Log.h"

namespace AF {
namespace RHI {

// ── 格式转换帮助函数 ──

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

// ── GLTexture2D ──

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

    // DSA 创建并分配不可变存储
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

    // 默认采样/包裹参数
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

    // 根据通道数推断纹理格式
    switch (channels)
    {
        case 1: m_Format = TextureFormat::R8;    break;
        case 3: m_Format = TextureFormat::RGB8;  break;
        case 4: default: m_Format = TextureFormat::RGBA8; break;
    }
    GLFormatFromTextureFormat(m_Format, m_InternalFormat, m_DataFormat);

    // DSA 创建并填充数据
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

// ── 工厂 ──

Ref<RHITexture2D> RHITexture2D::Create(uint32_t width, uint32_t height, TextureFormat format)
{
    return std::make_shared<GLTexture2D>(width, height, format);
}

Ref<RHITexture2D> RHITexture2D::Create(const std::string& path)
{
    return std::make_shared<GLTexture2D>(path);
}

// ── GLTextureCube ──

GLTextureCube::GLTextureCube(uint32_t size, TextureFormat format)
{
    CreateBlank(size, format);
}

GLTextureCube::GLTextureCube(const std::array<std::string, 6>& facePaths)
{
    LoadFromFaces(facePaths);
}

GLTextureCube::~GLTextureCube()
{
    glDeleteTextures(1, &m_RendererID);
}

void GLTextureCube::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_RendererID);
}

void GLTextureCube::CreateBlank(uint32_t size, TextureFormat format)
{
    m_Size = size;
    m_Format = format;
    GLFormatFromTextureFormat(format, m_InternalFormat, m_DataFormat);

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Size, m_Size);

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    AF_LOG_INFO("Created blank cubemap texture ({}x{})", m_Size, m_Size);
}

void GLTextureCube::LoadFromFaces(const std::array<std::string, 6>& facePaths)
{
    // 立方体贴图不需要翻转 Y 轴（GL 约定 +Y 向上）
    stbi_set_flip_vertically_on_load(0);

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);

    bool firstFace = true;

    // 按顺序加载 6 个面：+X -X +Y -Y +Z -Z
    for (int i = 0; i < 6; ++i)
    {
        int width, height, channels;
        unsigned char* data = stbi_load(facePaths[i].c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            AF_LOG_ERROR("Failed to load cubemap face: {}", facePaths[i]);
            continue;
        }

        if (m_Size == 0)
            m_Size = static_cast<uint32_t>(width);

        if (firstFace)
        {
            // 以第一张脸确定纹理格式
            switch (channels)
            {
                case 1: m_Format = TextureFormat::R8;    break;
                case 3: m_Format = TextureFormat::RGB8;  break;
                case 4: default: m_Format = TextureFormat::RGBA8; break;
            }
            GLFormatFromTextureFormat(m_Format, m_InternalFormat, m_DataFormat);
            glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Size, m_Size);
            firstFace = false;
        }

        // 使用 glTextureSubImage3D 将面数据上传到对应层（zoffset = face index）
        glTextureSubImage3D(m_RendererID, 0, 0, 0, i, m_Size, m_Size, 1, m_DataFormat, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateTextureMipmap(m_RendererID);

    AF_LOG_INFO("Loaded cubemap from 6 faces ({}x{})", m_Size, m_Size);
}

Ref<RHITextureCube> RHITextureCube::Create(uint32_t size, TextureFormat format)
{
    return std::make_shared<GLTextureCube>(size, format);
}

Ref<RHITextureCube> RHITextureCube::Create(const std::array<std::string, 6>& facePaths)
{
    return std::make_shared<GLTextureCube>(facePaths);
}

} // namespace RHI
} // namespace AF
