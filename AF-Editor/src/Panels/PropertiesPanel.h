#pragma once

#include "AF/Core/Base.h"
#include "AF/Scene/Entity.h"

namespace AF {

	/**
	 * @brief 属性面板 (Properties Panel)
	 * 负责显示和编辑当前选中实体的所有组件属性。
	 */
	class PropertiesPanel
	{
	public:
		PropertiesPanel() = default;
		
		/**
		 * @brief 设置当前处理的上下文
		 */
		void SetContext(Entity entity);

		/**
		 * @brief 渲染 ImGui 界面
		 */
		void OnImGuiRender();

	private:
		/**
		 * @brief 绘制所有组件
		 */
		void DrawComponents(Entity entity);

	private:
		Entity m_SelectionContext; // 当前选中的实体
	};

}
