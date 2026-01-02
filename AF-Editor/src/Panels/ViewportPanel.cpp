#include "ViewportPanel.h"

#include <glm/gtc/type_ptr.hpp>

#include "AF/Math/Math.h"
#include "AF/Renderer/SceneRenderer.h"
#include "AF/Core/Input.h"
#include "AF/Core/KeyCodes.h"
#include "AF/Scene/Components.h"

#include "ImGuizmo.h"

namespace AF {

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 视口渲染主循环：负责绘制场景纹理、处理拖放和调用 Gizmo 绘制
	void ViewportPanel::OnImGuiRender(const Ref<Scene>& activeScene,
		Ref<EditorCamera>& editorCamera,
		Entity selectedEntity,
		int& gizmoType,
		bool showPhysicsColliders,
		std::function<void(const std::filesystem::path&)> onSceneOpen)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		// 1. 更新视口几何信息 (用于 Gizmo 定位)
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		// 2. 更新焦点和悬停状态 (用于输入控制阻止)
		m_Focused = ImGui::IsWindowFocused();
		m_Hovered = ImGui::IsWindowHovered();

		// 3. 更新视口尺寸
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		// 4. 渲染场景纹理
		// 获取当前渲染管线的最终输出缓冲纹理 ID
		uint64_t textureID = SceneRenderer::GetBufferRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), 
			ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, 
			ImVec2{ 0, 1 }, 
			ImVec2{ 1, 0 });

		// 5. 处理文件拖放 (Drag & Drop)
		// 允许从内容浏览器拖拽场景文件 (*.af) 到视口直接打开
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				if (onSceneOpen)
					onSceneOpen(path);
			}
			ImGui::EndDragDropTarget();
		}

		// 6. 绘制并处理 Gizmos (变换工具)
		DrawGizmos(editorCamera, selectedEntity, gizmoType);

		ImGui::End();
		ImGui::PopStyleVar();
	}

	// ===================================================================================
	// --- 私有辅助 (Private Helpers) ---
	// ===================================================================================

	// 绘制和处理 Gizmo 操作 (移动/旋转/缩放)
	void ViewportPanel::DrawGizmos(Ref<EditorCamera>& editorCamera, Entity selectedEntity, int& gizmoType)
	{
		// 仅当选中实体且 Gizmo 类型有效时才绘制
		if (selectedEntity && gizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			// 设置 Gizmo 绘制区域与视口重合
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
				m_ViewportBounds[1].x - m_ViewportBounds[0].x,
				m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// 获取编辑器相机的视图和投影矩阵
			const glm::mat4& cameraProjection = editorCamera->GetProjection();
			glm::mat4 cameraView = editorCamera->GetViewMatrix();

			// 获取选中实体的当前变换矩阵
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// 处理吸附 (Snapping) - 按住 Ctrl 键开启
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // 位移/缩放吸附步长 (0.5m)
			if (gizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f; // 旋转吸附步长 (45度)

			float snapValues[3] = { snapValue, snapValue, snapValue };

			// 绘制 Gizmo 控件并处理用户交互
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)gizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			// 如果用户正在拖拽 Gizmo，反解矩阵并回写到实体的 Transform 组件
			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				// 注意：这里需要计算旋转增量，以避免欧拉角死锁 (Gimbal Lock) 问题
				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}
	}

}
