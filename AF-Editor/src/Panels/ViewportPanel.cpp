#include "Panels/ViewportPanel.h"

#include <imgui.h>

namespace AF {

void ViewportPanel::OnImGuiRender()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    auto size = ImGui::GetContentRegionAvail();
    m_ContentWidth  = static_cast<uint32_t>(size.x > 0 ? size.x : 1);
    m_ContentHeight = static_cast<uint32_t>(size.y > 0 ? size.y : 1);

    if (m_RenderTexture)
    {
        ImGui::Image(
            (ImTextureID)(uintptr_t)m_RenderTexture->GetRendererID(),
            ImVec2(static_cast<float>(m_ContentWidth), static_cast<float>(m_ContentHeight)),
            ImVec2(0, 1), ImVec2(1, 0));
    }

    m_Hovered = ImGui::IsWindowHovered();

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace AF
