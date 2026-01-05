#pragma once

#include <imgui.h>
#include <string>

namespace AF {

	/**
	 * @brief 设置面板 (Settings Panel)
	 * 负责显示编辑器全局设置，如物理调试绘制开关等。
	 */
	class SettingsPanel
	{
	public:
		SettingsPanel() = default;

		/**
		 * @brief 渲染 ImGui 设置面板
		 * @param showPhysicsColliders 是否显示物理碰撞体 (引用，可修改)
		 * @param debugTextureName 当前选中的调试纹理名称 (引用，可修改)
		 */
		void OnImGuiRender(bool& showPhysicsColliders, std::string& debugTextureName);
	};
}
