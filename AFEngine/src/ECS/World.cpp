#include "ECS/World.h"
#include "ECS/Components.h"

#include "Core/Assert.h"
#include "Core/Log.h"

namespace AF {

Entity::Entity(entt::entity handle, World* world)
    : m_Handle(handle)
    , m_Registry(world ? &world->GetRegistry() : nullptr)
{
}

UUID Entity::GetUUID() const
{
    return GetComponent<IDComponent>().ID;
}

const std::string& Entity::GetName() const
{
    return GetComponent<TagComponent>().Tag;
}

World::World()
{
    AF_LOG_INFO("World initialized");
}

World::~World()
{
    m_Registry.clear();
    m_EntityMap.clear();
}

Entity World::CreateEntity(const std::string& name)
{
    // 创建 enTT 实体，附加 ID/Tag/Transform 组件
    entt::entity handle = m_Registry.create();
    UUID uuid;

    m_Registry.emplace<IDComponent>(handle, uuid);
    auto& idComp = m_Registry.get<IDComponent>(handle);
    idComp.CreationOrder = m_NextCreationOrder++;
    m_Registry.emplace<TagComponent>(handle, name);
    m_Registry.emplace<TransformComponent>(handle);

    m_EntityMap[uuid] = handle;

    AF_LOG_INFO("Entity created: {} ({})", name, static_cast<uint64_t>(uuid));

    return Entity(handle, this);
}

void World::DestroyEntity(Entity entity)
{
    if (!IsValid(entity)) return;

    UUID uuid = entity.GetUUID();
    m_EntityMap.erase(uuid);
    m_Registry.destroy(static_cast<entt::entity>(entity));

    AF_LOG_INFO("Entity destroyed: {}", static_cast<uint64_t>(uuid));
}

Entity World::GetEntity(UUID uuid)
{
    auto it = m_EntityMap.find(uuid);
    if (it != m_EntityMap.end() && m_Registry.valid(it->second))
    {
        return Entity(it->second, this);
    }
    return Entity();
}

bool World::IsValid(Entity entity) const
{
    return entity && m_Registry.valid(static_cast<entt::entity>(entity));
}

} // namespace AF
