#pragma once

// 组件定义 —— 引擎内建 ECS 组件
//
// 每个 Component 是纯数据结构（POD），附着在 Entity 上。
// TransformComponent.Rotation 使用 glm::quat（弧度制），非 vec3 Euler。

#include "Core/UUID.h"
#include "Core/Types.h"
#include "Material/MaterialInstance.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <string>

namespace AF {

/// 唯一 ID 组件（每个 Entity 创建时自动添加）
struct IDComponent
{
    UUID ID;
    uint32_t CreationOrder = 0;

    IDComponent() = default;
};

/// 标签组件（实体名称）
struct TagComponent
{
    std::string Tag;

    TagComponent() = default;
    TagComponent(const std::string& tag) : Tag(tag) {}
};

/// 变换组件（位置/旋转/缩放）
///
/// 获取世界矩阵用 GetTransform()，旋转始终为四元数。
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

/// 网格组件（引用 Mesh 资源）
struct MeshComponent
{
    Ref<class Mesh> Source;

    MeshComponent() = default;
    MeshComponent(const Ref<Mesh>& mesh) : Source(mesh) {}
};

/// 材质组件（引用 MaterialInstance）
struct MaterialComponent
{
    Ref<class MaterialInstance> Source;
    uint64_t ActiveVariant = 0;

    MaterialComponent() = default;
    MaterialComponent(const Ref<MaterialInstance>& matInst) : Source(matInst) {}
};

/// 光源组件
struct LightComponent
{
    glm::vec3 Color     = { 1.0f, 1.0f, 1.0f };
    float Intensity     = 1.0f;
    uint32_t Type       = 0;  // 0=Directional, 1=Point
};

/// 相机组件
struct CameraComponent
{
    Ref<class Camera> Source;
    bool Primary = true;  // 标记为主相机，渲染管线优先使用
};

} // namespace AF
