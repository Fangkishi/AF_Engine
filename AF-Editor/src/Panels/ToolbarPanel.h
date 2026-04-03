#pragma once

#include "AF/Renderer/Texture.h"
#include <imgui.h>

namespace AF {

	/**
	 * @brief 工具栏面板 (Toolbar Panel)
	 * 负责顶部工具栏的渲染，包含播放、暂停、模拟等控制按钮。
	 */
	class ToolbarPanel
	{
	public:
		ToolbarPanel() = default;

		/**
		 * @brief 初始化资源 (加载图标)
		 */
		void OnAttach();

		/**
		 * @brief 渲染 ImGui 界面
		 * @param sceneState 当前场景状态 (0: Edit, 1: Play, 2: Simulate)
		 * @param isPaused 当前是否暂停
		 * @param onPlay 播放回调
		 * @param onSimulate 模拟回调
		 * @param onStop 停止回调
		 * @param onPause 暂停回调
		 * @param onStep 单步回调
		 */
		void OnImGuiRender(int sceneState, bool isPaused,
			std::function<void()> onPlay,
			std::function<void()> onSimulate,
			std::function<void()> onStop,
			std::function<void()> onPause,
			std::function<void()> onStep);

	private:
		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconPause;
		Ref<Texture2D> m_IconStep;
		Ref<Texture2D> m_IconSimulate;
		Ref<Texture2D> m_IconStop;
	};
}
