#pragma once

#include <UI/ImGuiSystem.h>

#include "EditorCamera.h"
#include "Panels/Panel.h"

#include <memory>
#include <vector>

namespace AF {

class ViewportPanel;
class DeferredRenderPipeline;

class EditorSystem : public ImGuiSystem
{
public:
    void OnInitialize(Engine& engine) override;
    void OnUpdate(float dt) override;
    void OnImGui() override;

private:
    void DrawMenuBar();

    EditorCamera m_EditorCamera;
    std::vector<std::unique_ptr<Panel>> m_Panels;
    ViewportPanel* m_ViewportPanel = nullptr;
    uint32_t m_LastViewportW = 0;
    uint32_t m_LastViewportH = 0;
};

} // namespace AF
