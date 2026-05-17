#pragma once

// Entity —— 游戏实体包装类
//
// 对 entt::entity 和 entt::registry 指针的轻量级封装。
// 提供类型安全的 AddComponent / GetComponent / HasComponent / RemoveComponent。
// Entity 通过 enTT ECS 管理，id 为 uint32_t。

#include "ECS/Components.h"
#include "Core/Assert.h"

#include <entt.hpp>
#include <cstdint>

namespace AF {

class World;

class Entity
{
public:
    Entity() = default;
    Entity(entt::entity handle, World* world);
    Entity(const Entity& other) = default;

    /// 添加组件（断言确保不重复）
    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        AF_CORE_ASSERT(!HasComponent<T>(), "Entity already has this component!");
        return m_Registry->emplace<T>(m_Handle, std::forward<Args>(args)...);
    }

    /// 添加或替换组件（若已存在则替换）
    template <typename T, typename... Args>
    T& AddOrReplaceComponent(Args&&... args)
    {
        return m_Registry->emplace_or_replace<T>(m_Handle, std::forward<Args>(args)...);
    }

    /// 获取组件（断言确保存在）
    template <typename T>
    T& GetComponent()
    {
        AF_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
        return m_Registry->get<T>(m_Handle);
    }

    template <typename T>
    const T& GetComponent() const
    {
        AF_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
        return m_Registry->get<T>(m_Handle);
    }

    /// 检查是否拥有某组件
    template <typename T>
    bool HasComponent() const
    {
        return m_Registry->all_of<T>(m_Handle);
    }

    /// 移除组件
    template <typename T>
    void RemoveComponent()
    {
        AF_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
        m_Registry->remove<T>(m_Handle);
    }

    UUID GetUUID() const;
    const std::string& GetName() const;

    operator bool() const { return m_Handle != entt::null; }
    operator entt::entity() const { return m_Handle; }
    operator uint32_t() const { return static_cast<uint32_t>(m_Handle); }

    bool operator==(const Entity& other) const { return m_Handle == other.m_Handle && m_Registry == other.m_Registry; }
    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::entity m_Handle = entt::null;
    entt::registry* m_Registry = nullptr;
};

} // namespace AF
