#pragma once

#include "Core/Types.h"
#include "Renderer/Mesh.h"
#include "Renderer/Material.h"

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace AF {

struct EntitySnapshot
{
    glm::mat4       Transform     = glm::mat4(1.0f);
    Ref<Mesh>       MeshSource;
    Ref<Material>   MaterialSource;
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
    std::vector<EntitySnapshot> Entities;
    std::vector<LightData>       Lights;
};

} // namespace AF
