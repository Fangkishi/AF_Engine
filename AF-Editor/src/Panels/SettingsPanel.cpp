#include "SettingsPanel.h"
#include "AF/Renderer/SceneRenderer.h"

namespace AF {

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 渲染设置面板
	void SettingsPanel::OnImGuiRender(bool& showPhysicsColliders, std::string& debugTextureName)
	{
		ImGui::Begin("Settings");
		
		// 物理调试选项
		ImGui::Checkbox("Show physics colliders", &showPhysicsColliders);

		ImGui::Separator();
		ImGui::Text("Render Graph Debug");

		// 显示当前选中的纹理名称
		if (debugTextureName.empty())
			ImGui::Text("Current View: FinalColor");
		else
			ImGui::Text("Current View: %s", debugTextureName.c_str());

		if (ImGui::Button("Reset View"))
		{
			debugTextureName = "";
		}

		ImGui::Separator();

		// 获取所有渲染节点
		const auto& nodes = SceneRenderer::GetRenderNodes();

		if (ImGui::BeginCombo("Render Pass", "Select Pass..."))
		{
			for (const auto& node : nodes)
			{
				if (ImGui::BeginMenu(node.name.c_str()))
				{
					// 列出该节点的所有输出资源
					for (const auto& output : node.outputs)
					{
						if (ImGui::MenuItem(output.c_str(), nullptr, debugTextureName == output))
						{
							debugTextureName = output;
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndCombo();
		}
		
		ImGui::End();
	}

}
