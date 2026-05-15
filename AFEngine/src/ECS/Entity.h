#pragma once

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

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        AF_CORE_ASSERT(!HasComponent<T>(), "Entity already has this component!");
        return m_Registry->emplace<T>(m_Handle, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T& AddOrReplaceComponent(Args&&... args)
    {
        return m_Registry->emplace_or_replace<T>(m_Handle, std::forward<Args>(args)...);
    }

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

    template <typename T>
    bool HasComponent() const
    {
        return m_Registry->all_of<T>(m_Handle);
    }

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
