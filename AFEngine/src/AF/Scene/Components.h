#pragma once

#include "AF/Core/UUID.h"
#include "AF/Renderer/Texture.h"
#include "AF/Scene/SceneCamera.h"
#include "AF/Renderer/Shader.h"
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

		// 缂傛挸鐡ㄩ崣妯诲床閻晠妯€
		glm::mat4 Transform = glm::mat4(1.0f);
		glm::mat3 NormalMatrix = glm::mat3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{
		}

		/** @brief 閺囧瓨鏌婇獮鎯板箯閸欐牕褰夐幑銏㈢叐闂?*/
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
		UniformContainer instanceUniforms; ///< 鐎圭偘绶ョ粔浣规箒閻?Uniform 鐎圭懓娅掗敍鍫㈠缁斿绱?
		int EntityID = -1; // 鐎涙ê鍋嶇€圭偘缍?ID 閻ㄥ嫬澹囬張顒婄礉閻劋绨崷?UniformContainer 娑擃厼绱╅悽銊旂€规氨娈戦崷鏉挎絻

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;

		/** @brief 缂佹垵鐣炬径鏍劥缂佸嫪娆㈠鏇犳暏閿涘苯缂撶粩?Uniform 閸忓疇浠?*/
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

	// 楠炲疇顢戦崗澶岀矋娴?
	struct DirectionalLightComponent
	{
		glm::vec3 Ambient = glm::vec3(0.2f);
		glm::vec3 Diffuse = glm::vec3(1.5f);
		glm::vec3 Specular = glm::vec3(1.0f);
		bool Enabled = true;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	// 閻愮懓鍘滃┃鎰矋娴?
	struct PointLightComponent
	{
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		float Intensity = 5.0f;
		bool Enabled = true;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	template <typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<TransformComponent, SpriteRendererComponent, CircleRendererComponent, 
		CameraComponent, ScriptComponent, NativeScriptComponent, Rigidbody2DComponent,
		BoxCollider2DComponent, CircleCollider2DComponent, MeshComponent, MaterialComponent, 
		ParentChildComponent, DirectionalLightComponent, PointLightComponent>;

}
