#include "Panels/HierarchyPanel.h"

#include <imgui.h>

namespace AF {

void HierarchyPanel::OnImGuiRender()
{
    ImGui::Begin("Scene Hierarchy");

    // Placeholder — will iterate World entities
    ImGui::Text("(Entity list will appear here)");

    if (ImGui::Button("Create Entity"))
    {
        // TODO: Engine access for entity creation
    }

    ImGui::End();
}

} // namespace AF
