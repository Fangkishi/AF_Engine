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
#include <Renderer/RenderView.h>
#include <Renderer/Deferred/DeferredPipeline.h>
#include <Factory/MeshFactory.h>
#include <Factory/MaterialFactory.h>

#include "Panels/ViewportPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"

namespace AF {

void EditorSystem::OnInitialize(Engine& engine)
{
    ImGuiSystem::OnInitialize(engine);

    AF_LOG_INFO("EditorSystem: initializing...");

    auto vp = std::make_unique<ViewportPanel>();
    m_ViewportPanel = vp.get();
    m_Panels.push_back(std::move(vp));

    auto& world = engine.GetWorld();
    auto hp = std::make_unique<HierarchyPanel>(&world,
        [this](const std::string& type) { CreatePrimitive(type); },
        m_SelectedUUID);
    m_HierarchyPanel = hp.get();
    m_Panels.push_back(std::move(hp));

    auto ip = std::make_unique<InspectorPanel>(&world, m_SelectedUUID);
    m_InspectorPanel = ip.get();
    m_Panels.push_back(std::move(ip));

    AF_LOG_INFO("EditorSystem: {} panels registered", m_Panels.size());
}

void EditorSystem::OnUpdate(float dt)
{
    auto& engine = *GetEngine();
    auto& world = engine.GetWorld();

    if (m_ViewportPanel && m_ViewportPanel->IsHovered())
        m_EditorCamera.OnUpdate(dt, engine);

    uint32_t vpW = m_ViewportPanel ? m_ViewportPanel->GetContentWidth()  : 1;
    uint32_t vpH = m_ViewportPanel ? m_ViewportPanel->GetContentHeight() : 1;
    float aspect = (vpH > 0) ? static_cast<float>(vpW) / static_cast<float>(vpH) : 1.78f;

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

    RenderView rv;
    rv.Width  = vpW;
    rv.Height = vpH;

    if (!m_ActiveCameraUUID)
    {
        m_EditorCamera.GetCamera().SetAspectRatio(aspect);
        rv.Projection = m_EditorCamera.GetProjection();
        rv.View       = m_EditorCamera.GetView();
        rv.Position   = m_EditorCamera.GetPosition();
        rv.Forward    = m_EditorCamera.GetCameraForward();
    }
    else
    {
        auto entity = world.GetEntity(m_ActiveCameraUUID);
        if (entity && entity.HasComponent<CameraComponent>() && entity.HasComponent<TransformComponent>())
        {
            auto& camComp = entity.GetComponent<CameraComponent>();
            auto& transform = entity.GetComponent<TransformComponent>();
            camComp.Source->SetAspectRatio(aspect);
            camComp.Source->SetPosition(transform.Position);
            camComp.Source->SetRotation(transform.Rotation);
            rv.Projection = camComp.Source->GetProjection();
            rv.View       = camComp.Source->GetView();
            rv.Position   = transform.Position;
            rv.Forward    = camComp.Source->GetForward();
        }
        else
        {
            m_ActiveCameraUUID = UUID();
            m_EditorCamera.GetCamera().SetAspectRatio(aspect);
            rv.Projection = m_EditorCamera.GetProjection();
            rv.View       = m_EditorCamera.GetView();
            rv.Position   = m_EditorCamera.GetPosition();
            rv.Forward    = m_EditorCamera.GetCameraForward();
        }
    }

    rv.ViewProjection = rv.Projection * rv.View;
    engine.GetSystem<RenderSystem>().SetCameraView(rv);

    auto& pipeline = engine.GetSystem<DeferredRenderPipeline>();
    m_ViewportPanel->SetRenderTexture(pipeline.GetOutput("finalComposite"));

    m_ViewportPanel->SetCameraList(GetCameraNames(), GetActiveCameraIndex(),
        [this](int index) { SetActiveCameraIndex(index); });

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

        if (ImGui::BeginMenu("GameObject"))
        {
            if (ImGui::BeginMenu("3D Object"))
            {
                if (ImGui::MenuItem("Cube"))      CreatePrimitive("Cube");
                if (ImGui::MenuItem("Plane"))     CreatePrimitive("Plane");
                if (ImGui::MenuItem("Sphere"))    CreatePrimitive("Sphere");
                if (ImGui::MenuItem("Cylinder"))  CreatePrimitive("Cylinder");
                if (ImGui::MenuItem("Capsule"))   CreatePrimitive("Capsule");
                ImGui::EndMenu();
            }
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

void EditorSystem::SetActiveCameraIndex(int index)
{
    if (index <= 0 || GetCameraNames().size() <= 1)
    {
        m_ActiveCameraUUID = UUID();
        return;
    }

    auto& world = GetEngine()->GetWorld();
    auto view = world.View<CameraComponent>();
    int i = 1;
    for (auto [handle, camComp] : view.each())
    {
        (void)camComp;
        if (i == index)
        {
            Entity entity(handle, &world);
            m_ActiveCameraUUID = entity.GetUUID();
            AF_LOG_INFO("Switched to scene camera '{}'", entity.GetName());
            return;
        }
        i++;
    }
    m_ActiveCameraUUID = UUID();
}

std::vector<std::string> EditorSystem::GetCameraNames()
{
    std::vector<std::string> names;
    names.push_back("Editor Camera");

    auto& world = GetEngine()->GetWorld();
    auto view = world.View<TagComponent, CameraComponent>();
    for (auto [_, tag, camComp] : view.each())
    {
        (void)camComp;
        names.push_back(tag.Tag);
    }
    return names;
}

int EditorSystem::GetActiveCameraIndex()
{
    if (!m_ActiveCameraUUID)
        return 0;

    auto& world = GetEngine()->GetWorld();
    auto view = world.View<CameraComponent>();
    int idx = 1;
    for (auto [handle, camComp] : view.each())
    {
        (void)camComp;
        Entity entity(handle, &world);
        if (entity.GetUUID() == m_ActiveCameraUUID)
            return idx;
        idx++;
    }
    return 0;
}

Entity EditorSystem::CreatePrimitive(const std::string& type)
{
    auto& engine = *GetEngine();
    auto& world = engine.GetWorld();

    Ref<Mesh> mesh;
    if (type == "Cube")        mesh = MeshFactory::CreateCube();
    else if (type == "Plane")  mesh = MeshFactory::CreatePlane();
    else if (type == "Sphere") mesh = MeshFactory::CreateSphere();
    else if (type == "Cylinder") mesh = MeshFactory::CreateCylinder();
    else if (type == "Capsule") mesh = MeshFactory::CreateCapsule();
    else
    {
        AF_LOG_ERROR("CreatePrimitive: unknown type '{}'", type);
        return {};
    }

    std::string name = type;
    int suffix = 1;
    auto view = world.View<TagComponent>();
    for (;;)
    {
        bool exists = false;
        for (auto [_, tag] : view.each())
        {
            if (tag.Tag == name)
            {
                exists = true;
                break;
            }
        }
        if (!exists) break;
        name = type + " (" + std::to_string(suffix++) + ")";
    }

    auto entity = world.CreateEntity(name);
    entity.AddComponent<MeshComponent>(mesh);
    entity.AddComponent<MaterialComponent>(MaterialFactory::CreateDefault());

    AF_LOG_INFO("Created entity '{}' ({})", name, type);
    return entity;
}

} // namespace AF
