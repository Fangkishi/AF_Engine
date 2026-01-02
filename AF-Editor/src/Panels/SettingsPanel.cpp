#include "SettingsPanel.h"

namespace AF {

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 渲染设置面板
	void SettingsPanel::OnImGuiRender(bool& showPhysicsColliders)
	{
		ImGui::Begin("Settings");
		
		// 物理调试选项
		ImGui::Checkbox("Show physics colliders", &showPhysicsColliders);
		
		ImGui::End();
	}

}
