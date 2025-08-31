#include "afpch.h"
#include "Entity.h"

namespace AF {
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}
}
