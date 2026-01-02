#pragma once

#include "AF/Renderer/Renderer2D.h"
#include <imgui/imgui.h>
#include <string>

namespace AF {

	/**
	 * @brief 统计面板 (Stats Panel)
	 * 负责显示渲染器的性能统计信息，如 Draw Calls, 顶点数等。
	 */
	class StatsPanel
	{
	public:
		StatsPanel() = default;

		/**
		 * @brief 渲染 ImGui 统计面板
		 */
		void OnImGuiRender();
	};
}
