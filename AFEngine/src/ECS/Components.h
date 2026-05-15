#pragma once

#include "Core/UUID.h"
#include "Core/Types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <string>

namespace AF {

struct IDComponent
{
    UUID ID;

    IDComponent() = default;
};

struct TagComponent
{
    std::string Tag;

    TagComponent() = default;
    TagComponent(const std::string& tag) : Tag(tag) {}
};

struct TransformComponent
{
    glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
    glm::quat Rotation = glm::identity<glm::quat>();
    glm::vec3 Scale    = { 1.0f, 1.0f, 1.0f };

    TransformComponent() = default;

    glm::mat4 GetTransform() const
    {
        return glm::translate(glm::mat4(1.0f), Position)
             * glm::toMat4(Rotation)
             * glm::scale(glm::mat4(1.0f), Scale);
    }
};

struct MeshComponent
{
    Ref<class Mesh> Source;

    MeshComponent() = default;
    MeshComponent(const Ref<Mesh>& mesh) : Source(mesh) {}
};

struct MaterialComponent
{
    Ref<class Material> Source;

    MaterialComponent() = default;
    MaterialComponent(const Ref<Material>& mat) : Source(mat) {}
};

struct LightComponent
{
    glm::vec3 Color     = { 1.0f, 1.0f, 1.0f };
    float Intensity     = 1.0f;
    uint32_t Type       = 0;  // 0=Directional, 1=Point
};

struct CameraComponent
{
    Ref<class Camera> Source;
    bool Primary = true;
};

} // namespace AF
