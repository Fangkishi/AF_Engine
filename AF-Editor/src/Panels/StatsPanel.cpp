#include "StatsPanel.h"

namespace AF {

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 渲染统计面板
	void StatsPanel::OnImGuiRender()
	{
		ImGui::Begin("Stats");
		
		// 1. 获取并计算帧率
		// ImGui::GetIO().Framerate 提供了基于 ImGui 内部计时的平滑帧率，
		// 使用它不会与主循环的时间步长耦合，且平滑度更好。
		float framerate = ImGui::GetIO().Framerate;
		float frameTime = 1000.0f / (framerate > 0.001f ? framerate : 1.0f); // 毫秒

		ImGui::Text("Performance:");
		ImGui::Text("FPS: %.1f", framerate);
		ImGui::Text("Frame Time: %.3f ms", frameTime);
		
		ImGui::Separator();

		// 2. 获取 Renderer2D 的统计数据
		auto stats = Renderer2D::GetStats();
		
		// 显示统计信息
		ImGui::Text("Renderer Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

		ImGui::End();
	}

}
