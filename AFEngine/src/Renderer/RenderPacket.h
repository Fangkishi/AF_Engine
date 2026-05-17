#pragma once

// RenderPacket —— 每帧渲染数据包
//
// EntitySnapshot：网格实体快照（变换矩阵 + Mesh + MaterialInstance）
// LightData：光源数据（位置/方向/颜色/强度/类型）
// RenderSystem 每帧从 ECS 世界收集这些数据，供管线使用。

#include "Core/Types.h"
#include "Renderer/Mesh.h"
#include "Material/MaterialInstance.h"

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace AF {

struct EntitySnapshot
{
    glm::mat4              Transform        = glm::mat4(1.0f);
    Ref<Mesh>              MeshSource;
    Ref<MaterialInstance>  MaterialSource;
    uint64_t               MaterialVariant  = 0;
};

struct LightData
{
    glm::vec3 Position  = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Direction = { 0.0f, -1.0f, -1.0f };
    glm::vec3 Color     = { 1.0f, 1.0f, 1.0f };
    float Intensity     = 1.0f;
    uint32_t Type       = 0;
};

struct RenderPacket
{
    std::vector<EntitySnapshot> Opaque;
    std::vector<EntitySnapshot> Translucent;
    std::vector<LightData>      Lights;
};

} // namespace AF
