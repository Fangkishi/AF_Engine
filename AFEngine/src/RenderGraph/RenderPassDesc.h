#pragma once

#include "RHI/RHITypes.h"

#include <cstdint>
#include <vector>

namespace AF {

struct RenderPassDesc
{
    uint32_t Width  = 1920;
    uint32_t Height = 1080;
    std::vector<RHI::TextureFormat> Attachments;
    bool Clear = true;
};

inline bool IsDepthFormat(RHI::TextureFormat fmt)
{
    return fmt == RHI::TextureFormat::Depth32 || fmt == RHI::TextureFormat::Depth24Stencil8;
}

} // namespace AF
