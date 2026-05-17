#pragma once

// RenderView —— 每帧相机渲染视图数据
//
// 由 RenderSystem 每帧构建，传递给 RenderPipeline 和 RenderGraph。
// 包含投影/视图矩阵、相机位置和方向、视口尺寸。

#include <glm/glm.hpp>
#include <cstdint>

namespace AF {

struct RenderView
{
    glm::mat4 ViewProjection = glm::mat4(1.0f);
    glm::mat4 Projection     = glm::mat4(1.0f);
    glm::mat4 View           = glm::mat4(1.0f);
    glm::vec3 Position       = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Forward        = { 0.0f, 0.0f, -1.0f };
    uint32_t Width  = 1920;
    uint32_t Height = 1080;
};

} // namespace AF
