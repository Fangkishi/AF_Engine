#pragma once

#include "AF/Core/Base.h"
#include "AF/Scene/Scene.h"
#include "AF/Scene/Entity.h"

namespace AF {

	/**
	* @brief 场景层级面板
	* 负责显示场景中所有实体的树状结构，并允许选择实体。
	*/
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		/**
		 * @brief 设置当前面板处理的场景上下文
		 */
		void SetContext(const Ref<Scene>& scene);

		/**
		 * @brief 渲染 ImGui 界面
		 */
		void OnImGuiRender();

		/**
		 * @brief 获取当前选中的实体
		 */
		Entity GetSelectedEntity() const { return m_SelectionContext; }
		
		/**
		 * @brief 设置选中的实体
		 */
		void SetSelectedEntity(Entity entity);

	private:
		/**
		 * @brief 递归绘制实体节点
		 */
		void DrawEntityNode(Entity entity);

	private:
		Ref<Scene> m_Context;      // 当前场景
		Entity m_SelectionContext; // 当前选中的实体
	};

}
