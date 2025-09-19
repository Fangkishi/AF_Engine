#pragma once

#include "AF/Core/Timestep.h"
#include "AF/Core/UUID.h"
#include "AF/Renderer/EditorCamera.h"

#include "entt.hpp"

class b2World;

namespace AF {

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void UpdateScripts(Timestep ts);
		void UpdatePhysics(Timestep ts);

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity DuplicateEntity(Entity entity);

		Entity FindEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID uuid);

		void SetCamera(Ref<Camera> camera) { m_Camera = camera; }
		const Ref<Camera> GetCamera() const { return m_Camera; }

		Entity GetPrimaryCameraEntity();

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void SetPaused(bool paused) { m_IsPaused = paused; }

		void Step(int frames = 1);

		template<typename... Components>
		auto GetAllEntitiesWithView()
		{
			return m_Registry.view<Components...>();
		}

		template<typename... Components, typename... OtherComponents>
		auto GetAllEntitiesWithGroup(entt::get_t<OtherComponents...>)
		{
			return m_Registry.group<Components...>(entt::get<OtherComponents...>);
		}
	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		Ref<Camera> m_Camera;

		b2World* m_PhysicsWorld = nullptr;

		std::unordered_map<UUID, entt::entity> m_EntityMap;

		//friend class EditorLayer;
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}
