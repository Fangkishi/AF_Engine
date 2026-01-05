#pragma once

#include "AF/Scene/Scene.h"
#include "AF/Renderer/EditorCamera.h"
#include "AF/Scene/Entity.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <functional>
#include <filesystem>

namespace AF {

	/**
	 * @brief 视口面板 (Viewport Panel)
	 * 负责渲染场景的最终图像，处理 Gizmo 操作，以及文件拖放打开场景。
	 */
	class ViewportPanel
	{
	public:
		ViewportPanel() = default;

		/**
		 * @brief 渲染 ImGui 视口
		 * @param activeScene 当前活动场景
		 * @param editorCamera 编辑器相机
		 * @param selectedEntity 当前选中的实体
		 * @param gizmoType 当前 Gizmo 操作类型 (移动/旋转/缩放)
		 * @param showPhysicsColliders 是否显示物理碰撞体 (可选)
		 * @param onSceneOpen 打开场景的回调 (处理拖放)
		 */
		void OnImGuiRender(const Ref<Scene>& activeScene,
			Ref<EditorCamera>& editorCamera,
			Entity selectedEntity,
			int& gizmoType,
			bool showPhysicsColliders,
			std::function<void(const std::filesystem::path&)> onSceneOpen,
			const std::string& debugTextureName = "",
			int debugLayer = 0,
			int debugFace = 0);

		glm::vec2 GetSize() const { return m_ViewportSize; }
		glm::vec2 GetViewportMinBound() const { return m_ViewportBounds[0]; }
		glm::vec2 GetViewportMaxBound() const { return m_ViewportBounds[1]; }
		bool IsFocused() const { return m_Focused; }
		bool IsHovered() const { return m_Hovered; }

	private:
		// --- 辅助渲染函数 ---
		void DrawGizmos(Ref<EditorCamera>& editorCamera, Entity selectedEntity, int& gizmoType);

	private:
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2] = { {0.0f, 0.0f}, {0.0f, 0.0f} };
		bool m_Focused = false;
		bool m_Hovered = false;
	};
}
