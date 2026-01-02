#include "StatsPanel.h"

namespace AF {

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 渲染统计面板
	void StatsPanel::OnImGuiRender()
	{
		ImGui::Begin("Stats");
		
		// 获取 Renderer2D 的统计数据
		auto stats = Renderer2D::GetStats();
		
		// 显示统计信息
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

		ImGui::End();
	}

}
