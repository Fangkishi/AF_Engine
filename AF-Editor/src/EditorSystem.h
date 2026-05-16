#pragma once

#include <UI/ImGuiSystem.h>

#include "EditorCamera.h"
#include "Panels/Panel.h"

#include <Core/UUID.h>
#include <ECS/Entity.h>

#include <memory>
#include <string>
#include <vector>

namespace AF {

class ViewportPanel;
class DeferredRenderPipeline;
class HierarchyPanel;
class InspectorPanel;

class EditorSystem : public ImGuiSystem
{
public:
    void OnInitialize(Engine& engine) override;
    void OnUpdate(float dt) override;
    void OnImGui() override;

    void SetActiveCameraIndex(int index);
    std::vector<std::string> GetCameraNames();
    int GetActiveCameraIndex();

private:
    void DrawMenuBar();
    Entity CreatePrimitive(const std::string& type);

    EditorCamera m_EditorCamera;
    std::vector<std::unique_ptr<Panel>> m_Panels;
    ViewportPanel* m_ViewportPanel = nullptr;
    HierarchyPanel* m_HierarchyPanel = nullptr;
    InspectorPanel* m_InspectorPanel = nullptr;
    uint32_t m_LastViewportW = 0;
    uint32_t m_LastViewportH = 0;
    UUID m_ActiveCameraUUID;
    UUID m_SelectedUUID = UUID(0);
};

} // namespace AF
