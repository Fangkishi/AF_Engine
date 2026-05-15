#pragma once

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
