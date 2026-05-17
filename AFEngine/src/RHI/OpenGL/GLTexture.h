#pragma once

// GLTexture —— OpenGL 2D 纹理和立方体贴图实现
//
// 2D 纹理支持从文件加载（stb_image）和空白创建（渲染目标）。
// 立方体贴图支持从 6 张单面图片加载或空白创建（环境贴图渲染目标）。
// 均使用 DSA（glCreateTextures / glTextureStorage2D）API。

#include "RHI/RHITexture.h"

#include <array>
#include <string>

namespace AF {
namespace RHI {

class GLTexture2D : public RHITexture2D
{
public:
    GLTexture2D(uint32_t width, uint32_t height, TextureFormat format);
    GLTexture2D(const std::string& path);
    ~GLTexture2D() override;

    uint32_t GetWidth() const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }
    uint32_t GetRendererID() const override { return m_RendererID; }
    TextureFormat GetFormat() const override { return m_Format; }

    void Bind(uint32_t slot = 0) const override;

private:
    void CreateBlank(uint32_t width, uint32_t height, TextureFormat format);
    void LoadFromFile(const std::string& path);

    uint32_t m_RendererID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    uint32_t m_InternalFormat = 0;
    uint32_t m_DataFormat = 0;
    TextureFormat m_Format = TextureFormat::Unknown;
};

class GLTextureCube : public RHITextureCube
{
public:
    GLTextureCube(uint32_t size, TextureFormat format);
    GLTextureCube(const std::array<std::string, 6>& facePaths);
    ~GLTextureCube() override;

    uint32_t GetSize() const override { return m_Size; }
    uint32_t GetRendererID() const override { return m_RendererID; }
    TextureFormat GetFormat() const override { return m_Format; }

    void Bind(uint32_t slot = 0) const override;

private:
    void CreateBlank(uint32_t size, TextureFormat format);
    void LoadFromFaces(const std::array<std::string, 6>& facePaths);

    uint32_t m_RendererID = 0;
    uint32_t m_Size = 0;
    uint32_t m_InternalFormat = 0;
    uint32_t m_DataFormat = 0;
    TextureFormat m_Format = TextureFormat::Unknown;
};

} // namespace RHI
} // namespace AF
