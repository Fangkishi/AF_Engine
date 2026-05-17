#pragma once

// RenderPassDesc —— 渲染通道描述
//
// 定义渲染通道的尺寸、颜色/深度附着格式和清除选项。
// 管线子类在 OnRender 中构建此描述，传入 RenderGraph::AddNode。

#include "RHI/RHITypes.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

namespace AF {

struct RenderPassDesc
{
    uint32_t Width  = 1920;
    uint32_t Height = 1080;
    std::vector<RHI::TextureFormat> Attachments;
    bool Clear = true;
    glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
};

/// 判断纹理格式是否为深度（或深度＋模板）格式
inline bool IsDepthFormat(RHI::TextureFormat fmt)
{
    return fmt == RHI::TextureFormat::Depth32 || fmt == RHI::TextureFormat::Depth24Stencil8;
}

} // namespace AF
