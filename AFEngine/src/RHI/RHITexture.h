#pragma once

#include "Core/Types.h"
#include "RHI/RHITypes.h"

#include <cstdint>
#include <string>

namespace AF {
namespace RHI {

class RHITexture2D
{
public:
    virtual ~RHITexture2D() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetRendererID() const = 0;

    virtual void Bind(uint32_t slot = 0) const = 0;

    virtual TextureFormat GetFormat() const = 0;

    static Ref<RHITexture2D> Create(uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8);
    static Ref<RHITexture2D> Create(const std::string& path);
};

} // namespace RHI
} // namespace AF
