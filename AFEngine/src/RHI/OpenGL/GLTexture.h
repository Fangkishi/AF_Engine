#pragma once

#include "RHI/RHITexture.h"

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

} // namespace RHI
} // namespace AF
