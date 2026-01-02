#include "ToolbarPanel.h"

namespace AF {

	// ===================================================================================
	// --- 生命周期 (Lifecycle) ---
	// ===================================================================================

	// 面板初始化 (加载图标资源)
	void ToolbarPanel::OnAttach()
	{
		// 加载编辑器按钮图标资源
		m_IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
		m_IconPause = Texture2D::Create("Resources/Icons/PauseButton.png");
		m_IconSimulate = Texture2D::Create("Resources/Icons/SimulateButton.png");
		m_IconStep = Texture2D::Create("Resources/Icons/StepButton.png");
		m_IconStop = Texture2D::Create("Resources/Icons/StopButton.png");
	}

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 渲染工具栏面板主逻辑
	void ToolbarPanel::OnImGuiRender(int sceneState, bool isPaused,
		std::function<void()> onPlay,
		std::function<void()> onSimulate,
		std::function<void()> onStop,
		std::function<void()> onPause,
		std::function<void()> onStep)
	{
		// 1. 设置工具栏样式 (无边框、无滚动条)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		// 2. 确定工具栏状态
		// TODO: 工具栏是否可用 (通常依赖于是否有活动场景)
		bool toolbarEnabled = true; 
		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5f;

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		// 状态映射常量
		const int State_Edit = 0;
		const int State_Play = 1;
		const int State_Simulate = 2;

		bool hasPlayButton = sceneState == State_Edit || sceneState == State_Play;
		bool hasSimulateButton = sceneState == State_Edit || sceneState == State_Simulate;
		bool hasPauseButton = sceneState != State_Edit;

		// 3. 绘制播放/停止按钮
		if (hasPlayButton)
		{
			// 根据当前状态切换图标 (Play -> Stop)
			Ref<Texture2D> icon = (sceneState == State_Edit || sceneState == State_Simulate) ? m_IconPlay : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (sceneState == State_Edit || sceneState == State_Simulate)
					onPlay();
				else if (sceneState == State_Play)
					onStop();
			}
		}

		// 4. 绘制模拟/停止按钮
		if (hasSimulateButton)
		{
			if (hasPlayButton)
				ImGui::SameLine();

			// 根据当前状态切换图标 (Simulate -> Stop)
			Ref<Texture2D> icon = (sceneState == State_Edit || sceneState == State_Play) ? m_IconSimulate : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (sceneState == State_Edit || sceneState == State_Play)
					onSimulate();
				else if (sceneState == State_Simulate)
					onStop();
			}
		}

		// 5. 绘制暂停/单步执行按钮
		if (hasPauseButton)
		{
			ImGui::SameLine();
			{
				Ref<Texture2D> icon = m_IconPause;
				if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
				{
					onPause();
				}
			}

			// 如果已暂停，显示单步按钮 (Next Frame)
			if (isPaused)
			{
				ImGui::SameLine();
				{
					Ref<Texture2D> icon = m_IconStep;
					if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
					{
						onStep();
					}
				}
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

}
