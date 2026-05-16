#include "Panels/ViewportPanel.h"

#include <imgui.h>

namespace AF {

void ViewportPanel::SetCameraList(const std::vector<std::string>& names, int activeIndex,
                                   std::function<void(int)> onSelect)
{
    m_CameraNames = names;
    m_ActiveCameraIndex = activeIndex;
    m_OnCameraSelect = std::move(onSelect);
}

void ViewportPanel::OnImGuiRender()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(GetName());

    if (!m_CameraNames.empty() && m_ActiveCameraIndex >= 0 &&
        static_cast<size_t>(m_ActiveCameraIndex) < m_CameraNames.size())
    {
        ImGui::SetNextItemWidth(160.0f);
        if (ImGui::BeginCombo("##camera", m_CameraNames[m_ActiveCameraIndex].c_str()))
        {
            for (int i = 0; i < static_cast<int>(m_CameraNames.size()); i++)
            {
                if (ImGui::Selectable(m_CameraNames[i].c_str(), i == m_ActiveCameraIndex))
                {
                    m_ActiveCameraIndex = i;
                    if (m_OnCameraSelect)
                        m_OnCameraSelect(i);
                }
            }
            ImGui::EndCombo();
        }
    }

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
