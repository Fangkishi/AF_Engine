#pragma once

// World —— ECS 世界的核心管理器
//
// 基于 enTT 实现实体创建/销毁和组件视图查询。
// 维护 UUID → entt::entity 映射，支持按 UUID 查找实体。

#include "ECS/Entity.h"
#include "Core/Types.h"

#include <entt.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace AF {

class World : public NonCopyable
{
public:
    World();
    ~World();

    /// 创建一个新实体，自动附加 IDComponent / TagComponent / TransformComponent
    Entity CreateEntity(const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    /// 按 UUID 查询实体（不存在时返回空 Entity）
    Entity GetEntity(UUID uuid);
    bool IsValid(Entity entity) const;

    /// 获取持有指定组件集合的视图
    template <typename... Components>
    auto View()
    {
        return m_Registry.view<Components...>();
    }

    /// 获取分组视图（性能优于 view，适用于频繁迭代的场景）
    template <typename... Components, typename... Other>
    auto Group(entt::get_t<Other...> get)
    {
        return m_Registry.group<Components...>(get);
    }

    entt::registry& GetRegistry() { return m_Registry; }
    const entt::registry& GetRegistry() const { return m_Registry; }

private:
    entt::registry m_Registry;
    std::unordered_map<UUID, entt::entity> m_EntityMap;
    uint32_t m_NextCreationOrder = 0;
};

} // namespace AF
