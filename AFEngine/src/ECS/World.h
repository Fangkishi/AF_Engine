#pragma once

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

    Entity CreateEntity(const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    Entity GetEntity(UUID uuid);
    bool IsValid(Entity entity) const;

    template <typename... Components>
    auto View()
    {
        return m_Registry.view<Components...>();
    }

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
};

} // namespace AF
