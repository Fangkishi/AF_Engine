#pragma once

#include "AF/Core/UUID.h"
#include "AF/Renderer/API/Texture.h"
#include "AF/Scene/SceneCamera.h"
#include "AF/Renderer/API/Shader.h"
#include "AF/Renderer/Mesh.h"
#include "AF/Renderer/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace AF {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;

		TagComponent(const std::string& tag)
			: Tag(tag)
		{
		}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

		// 缓存变换矩阵
		glm::mat4 Transform = glm::mat4(1.0f);
		glm::mat3 NormalMatrix = glm::mat3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{
		}

		/** @brief 更新并获取变换矩阵 */
		const glm::mat4& UpdateTransform()
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			Transform = glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);

			NormalMatrix = glm::transpose(glm::inverse(glm::mat3(Transform)));

			return Transform;
		}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;

		SpriteRendererComponent(const glm::vec4& color)
			: Color(color)
		{
		}
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct CameraComponent
	{
		Ref<SceneCamera> Camera = CreateRef<SceneCamera>();
		bool Primary = true; // TODO: think about moving to Scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Forward declaration
	class ScriptableEntity;

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete static_cast<T*>(nsc->Instance); nsc->Instance = nullptr; };
		}
	};

	// 2D Physics

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		// TODO(Yan): move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO(Yan): move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	// 3D

	struct MeshComponent
	{
		Ref<Mesh> mesh;
		UniformContainer instanceUniforms; ///< 实例私有的 Uniform 容器（独立）
		int EntityID = -1; // 存储实体 ID 的副本，用于在 UniformContainer 中引用稳定的地址

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;

		/** @brief 绑定外部组件引用，建立 Uniform 关联 */
		void Bind(TransformComponent& transform, int entityID)
		{
			EntityID = entityID;
			instanceUniforms.SetUniform("u_Transform", &transform.Transform);
			instanceUniforms.SetUniform("u_NormalMatrix", &transform.NormalMatrix);
			instanceUniforms.SetUniform("u_EntityID", &EntityID);
		}
	};

	struct MaterialComponent
	{
		Ref<Material> material;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent&) = default;
	};

	struct ParentChildComponent
	{
		UUID ParentID;
		std::vector<UUID> ChildrenIDs;

		ParentChildComponent() = default;
		ParentChildComponent(const ParentChildComponent&) = default;

		void AddChild(UUID childID)
		{
			ChildrenIDs.push_back(childID);
		}

		void RemoveChild(UUID childID)
		{
			ChildrenIDs.erase(std::remove(ChildrenIDs.begin(), ChildrenIDs.end(), childID), ChildrenIDs.end());
		}
	};

	// 平行光组件
	struct DirectionalLightComponent
	{
		glm::vec3 Ambient = glm::vec3(0.2f);
		glm::vec3 Diffuse = glm::vec3(1.5f);
		glm::vec3 Specular = glm::vec3(1.0f);
		bool Enabled = true;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	// 点光源组件
	struct PointLightComponent
	{
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		float Intensity = 5.0f;
		bool Enabled = true;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	// 光照探针组件
	struct LightProbeComponent
	{
		// 3阶球谐函数系数，每个颜色通道(RGB)需要9个系数，共27个float
		// 存储顺序为：L00, L1-1, L10, L11, L2-2, L2-1, L20, L21, L22
		// 这里我们使用 9 个 vec3 来同时存储 RGB 三个通道的系数
		glm::vec3 SHCoefficients[9] = { glm::vec3(0.0f) };
		
		// 探针的影响半径或边界，可用于局部混合或剔除
		float Radius = 5.0f;
		
		bool IsBaked = false;

		LightProbeComponent() = default;
		LightProbeComponent(const LightProbeComponent&) = default;
	};

	template <typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<TransformComponent, SpriteRendererComponent, CircleRendererComponent, 
		CameraComponent, ScriptComponent, NativeScriptComponent, Rigidbody2DComponent,
		BoxCollider2DComponent, CircleCollider2DComponent, MeshComponent, MaterialComponent, 
		ParentChildComponent, DirectionalLightComponent, PointLightComponent, LightProbeComponent>;

}
