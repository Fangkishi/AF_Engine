#include "PropertiesPanel.h"
#include "AF/Scene/Components.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

namespace AF {

	// ===================================================================================
	// --- 生命周期 (Lifecycle) ---
	// ===================================================================================

	// 设置当前选中的实体上下文
	void PropertiesPanel::SetContext(Entity entity)
	{
		m_SelectionContext = entity;
	}

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 每帧渲染 ImGui 界面
	void PropertiesPanel::OnImGuiRender()
	{
		ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();
	}

	// ===================================================================================
	// --- 事件处理 (Event Handling) ---
	// ===================================================================================

	// 目前没有特定的事件回调

	// ===================================================================================
	// --- 私有辅助 (Private Helpers) ---
	// ===================================================================================

	/**
	 * @note 逻辑冗余说明：
	 * 目前在 DrawMaterialUI、DrawSpriteRendererUI 以及 DrawMeshUI 中存在大量重复的纹理/资源加载和拖拽逻辑。
	 * 建议后续重构为通用的资源选择器或拖拽处理器。
	 */

	// 绘制 Vec3 控制控件，包含 X/Y/Z 轴的重置按钮和拖动条
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		// 绘制 X 轴控制
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize)) values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// 绘制 Y 轴控制
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// 绘制 Z 轴控制
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	// 绘制 Transform 组件 UI
	static void DrawTransformUI(TransformComponent& component)
	{
		DrawVec3Control("Translation", component.Translation);
		glm::vec3 rotation = glm::degrees(component.Rotation);
		DrawVec3Control("Rotation", rotation);
		component.Rotation = glm::radians(rotation);
		DrawVec3Control("Scale", component.Scale, 1.0f);
	}

	// 绘制 Camera 组件 UI
	static void DrawCameraUI(CameraComponent& component)
	{
		auto& camera = component.Camera;

		ImGui::Checkbox("Primary", &component.Primary);

		const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
		const char* currentProjectionTypeString = projectionTypeStrings[(int)camera->GetProjectionType()];
		if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
		{
			for (int i = 0; i < 2; i++)
			{
				bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
				if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
				{
					currentProjectionTypeString = projectionTypeStrings[i];
					camera->SetProjectionType((SceneCamera::ProjectionType)i);
				}
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (camera->GetProjectionType() == SceneCamera::ProjectionType::Perspective)
		{
			float perspectiveVerticalFov = glm::degrees(camera->GetPerspectiveVerticalFOV());
			if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
				camera->SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

			float perspectiveNear = camera->GetPerspectiveNearClip();
			if (ImGui::DragFloat("Near", &perspectiveNear))
				camera->SetPerspectiveNearClip(perspectiveNear);

			float perspectiveFar = camera->GetPerspectiveFarClip();
			if (ImGui::DragFloat("Far", &perspectiveFar))
				camera->SetPerspectiveFarClip(perspectiveFar);
		}

		if (camera->GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
		{
			float orthoSize = camera->GetOrthographicSize();
			if (ImGui::DragFloat("Size", &orthoSize))
				camera->SetOrthographicSize(orthoSize);

			float orthoNear = camera->GetOrthographicNearClip();
			if (ImGui::DragFloat("Near", &orthoNear))
				camera->SetOrthographicNearClip(orthoNear);

			float orthoFar = camera->GetOrthographicFarClip();
			if (ImGui::DragFloat("Far", &orthoFar))
				camera->SetOrthographicFarClip(orthoFar);

			ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
		}
	}

	// 绘制 Material 组件 UI，支持纹理拖拽
	static void DrawMaterialUI(MaterialComponent& component)
	{
		if (component.material)
		{
			const float thumbnailSize = 64.0f;
			ImGui::Text("Diffuse Map");
			Ref<Texture> diffuseTexture;
			bool hasDiffuse = false;

			if (component.material->HasUniform("u_DiffuseMap"))
			{
				try {
					diffuseTexture = component.material->GetUniform<Ref<Texture>>("u_DiffuseMap");
					hasDiffuse = true;
				} catch (...) { hasDiffuse = false; }
			}

			if (hasDiffuse && diffuseTexture)
			{
				ImGui::ImageButton((ImTextureID)(uintptr_t)diffuseTexture->GetRendererID(), ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
			}
			else
			{
				ImGui::Button("##Drag Texture Here", ImVec2(thumbnailSize, thumbnailSize));
			}

			// 处理纹理拖拽接收
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath(path);
					Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded()) component.material->SetUniform("u_DiffuseMap", texture);
				}
				ImGui::EndDragDropTarget();
			}

			// Specular Map logic similar to Diffuse...
			// (注意：这里保留了原代码的注释，表示逻辑重复)
		}
		else
		{
			ImGui::Text("No material assigned");
		}
	}

	// 绘制 SpriteRenderer 组件 UI
	static void DrawSpriteRendererUI(SpriteRendererComponent& component)
	{
		ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
		ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path texturePath(path);
				Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
				if (texture->IsLoaded()) component.Texture = texture;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
	}

	// 绘制 CircleRenderer 组件 UI
	static void DrawCircleRendererUI(CircleRendererComponent& component)
	{
		ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
		ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
		ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
	}

	// 绘制 Rigidbody2D 组件 UI
	static void DrawRigidbody2DUI(Rigidbody2DComponent& component)
	{
		const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
		const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
		if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
		{
			for (int i = 0; i < 3; i++)
			{
				bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
				if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
				{
					currentBodyTypeString = bodyTypeStrings[i];
					component.Type = (Rigidbody2DComponent::BodyType)i;
				}
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
	}

	// 绘制 BoxCollider2D 组件 UI
	static void DrawBoxCollider2DUI(BoxCollider2DComponent& component)
	{
		ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
		ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
		ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
	}

	// 绘制 CircleCollider2D 组件 UI
	static void DrawCircleCollider2DUI(CircleCollider2DComponent& component)
	{
		ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
		ImGui::DragFloat("Radius", &component.Radius);
		ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
	}

	// 绘制 Mesh 组件 UI
	static void DrawMeshUI(MeshComponent& component)
	{
		ImGui::Button("Select Mesh", ImVec2(100.0f, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				// TODO: Handle mesh drop (处理 Mesh 拖放逻辑)
			}
			ImGui::EndDragDropTarget();
		}
	}

	// 绘制 DirectionalLight 组件 UI
	static void DrawDirectionalLightUI(DirectionalLightComponent& component)
	{
		ImGui::Checkbox("Enabled", &component.Enabled);
		ImGui::ColorEdit3("Ambient", glm::value_ptr(component.Ambient));
		ImGui::ColorEdit3("Diffuse", glm::value_ptr(component.Diffuse));
		ImGui::ColorEdit3("Specular", glm::value_ptr(component.Specular));
	}

	// 绘制 PointLight 组件 UI
	static void DrawPointLightUI(PointLightComponent& component)
	{
		ImGui::Checkbox("Enabled", &component.Enabled);
		ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
		ImGui::DragFloat("Intensity", &component.Intensity, 0.1f, 0.0f, 100.0f);
	}

	// 通用组件绘制模板，负责组件的折叠头、移除菜单等通用逻辑
	template <typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	// 显示添加组件的菜单项模板
	template <typename T>
	static void DisplayAddComponentEntry(Entity entity, const std::string& entryName)
	{
		if (!entity.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				entity.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	// 绘制实体的所有组件
	void PropertiesPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<CameraComponent>(entity, "Camera");
			DisplayAddComponentEntry<ScriptComponent>(entity, "Script");
			DisplayAddComponentEntry<SpriteRendererComponent>(entity, "Sprite Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>(entity, "Circle Renderer");
			DisplayAddComponentEntry<Rigidbody2DComponent>(entity, "Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>(entity, "Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>(entity, "Circle Collider 2D");
			DisplayAddComponentEntry<MeshComponent>(entity, "Mesh");
			DisplayAddComponentEntry<MaterialComponent>(entity, "Material");
			DisplayAddComponentEntry<DirectionalLightComponent>(entity, "Directional Light");
			DisplayAddComponentEntry<PointLightComponent>(entity, "Point Light");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, DrawTransformUI);
		DrawComponent<CameraComponent>("Camera", entity, DrawCameraUI);
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, DrawSpriteRendererUI);
		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, DrawCircleRendererUI);
		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, DrawRigidbody2DUI);
		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, DrawBoxCollider2DUI);
		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, DrawCircleCollider2DUI);
		DrawComponent<MeshComponent>("Mesh", entity, DrawMeshUI);
		DrawComponent<MaterialComponent>("Material", entity, DrawMaterialUI);
		DrawComponent<DirectionalLightComponent>("Directional Light", entity, DrawDirectionalLightUI);
		DrawComponent<PointLightComponent>("Point Light", entity, DrawPointLightUI);
	}

}
