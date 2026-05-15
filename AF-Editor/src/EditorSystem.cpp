#include "EditorSystem.h"

#include <imgui.h>

#include <Core/Engine.h>
#include <Core/Input.h>
#include <Core/Log.h>
#include <ECS/World.h>
#include <ECS/Entity.h>
#include <ECS/Components.h>
#include <Renderer/Camera.h>
#include <Renderer/Renderer.h>
#include <Renderer/Deferred/DeferredPipeline.h>

#include "Panels/ViewportPanel.h"
#include "Panels/HierarchyPanel.h"

namespace AF {

void EditorSystem::OnInitialize(Engine& engine)
{
    ImGuiSystem::OnInitialize(engine);

    AF_LOG_INFO("EditorSystem: initializing...");

    auto& world = engine.GetWorld();
    auto camEntity = world.CreateEntity("Editor Camera");
    camEntity.GetComponent<TransformComponent>().Position = { 0.0f, 0.0f, 5.0f };
    CameraComponent cc;
    cc.Source  = std::make_shared<Camera>();
    cc.Source->SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    cc.Primary = true;
    camEntity.AddComponent<CameraComponent>(cc);

    auto vp = std::make_unique<ViewportPanel>();
    m_ViewportPanel = vp.get();
    m_Panels.push_back(std::move(vp));
    m_Panels.push_back(std::make_unique<HierarchyPanel>());

    AF_LOG_INFO("EditorSystem: {} panels registered", m_Panels.size());
}

void EditorSystem::OnUpdate(float dt)
{
    auto& engine = *GetEngine();

    if (m_ViewportPanel && m_ViewportPanel->IsHovered())
        m_EditorCamera.OnUpdate(dt, engine);

    auto& world = engine.GetWorld();
    auto camView = world.View<TransformComponent, CameraComponent>();
    for (auto [enttHandle, transform, camComp] : camView.each())
    {
        if (!camComp.Primary || !camComp.Source) continue;

        transform.Position = m_EditorCamera.GetPosition();
        transform.Rotation = m_EditorCamera.GetRotation();
        break;
    }

    // Detect viewport size change (threshold > 2px to avoid jitter)
    uint32_t vpW = m_ViewportPanel ? m_ViewportPanel->GetContentWidth()  : 1;
    uint32_t vpH = m_ViewportPanel ? m_ViewportPanel->GetContentHeight() : 1;
    if (vpW > 2 && vpH > 2 &&
        (vpW != m_LastViewportW || vpH != m_LastViewportH))
    {
        m_LastViewportW = vpW;
        m_LastViewportH = vpH;

        auto& rs = engine.GetSystem<RenderSystem>();
        rs.SetViewport(vpW, vpH);

        auto& pipeline = engine.GetSystem<DeferredRenderPipeline>();
        pipeline.Invalidate();
    }

    // Set render texture for viewport panel
    auto& pipeline = engine.GetSystem<DeferredRenderPipeline>();
    m_ViewportPanel->SetRenderTexture(pipeline.GetOutput("finalComposite"));

    for (auto& panel : m_Panels)
        panel->OnUpdate(dt);

    ImGuiSystem::OnUpdate(dt);
}

void EditorSystem::OnImGui()
{
    DrawMenuBar();

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    for (auto& panel : m_Panels)
        panel->OnImGuiRender();
}

void EditorSystem::DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit", "Alt+F4"))
                GetEngine()->Quit();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            for (auto& panel : m_Panels)
                ImGui::MenuItem(panel->GetName(), nullptr);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

} // namespace AF
