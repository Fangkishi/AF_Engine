#include "Renderer/Renderer.h"

#include "Core/Engine.h"
#include "Core/Log.h"
#include "Core/Log.h"
#include "ECS/World.h"
#include "ECS/Entity.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "RHI/RHIDevice.h"

namespace AF {

void RenderSystem::OnInitialize(Engine& engine)
{
    (void)engine;
    AF_LOG_INFO("RenderSystem initialized");
}

void RenderSystem::SetViewport(uint32_t width, uint32_t height)
{
    m_ViewportWidth  = width;
    m_ViewportHeight = height;

    auto& world = GetEngine()->GetWorld();
    auto camView = world.View<TransformComponent, CameraComponent>();
    for (auto [enttHandle, transform, camComp] : camView.each())
    {
        if (!camComp.Primary || !camComp.Source) continue;

        float aspect = static_cast<float>(width) / static_cast<float>(height);
        camComp.Source->SetAspectRatio(aspect);
        break;
    }
}

void RenderSystem::SetCameraView(const RenderView& view)
{
    m_View = view;
    m_CameraOverride = true;
}

void RenderSystem::OnUpdate(float dt)
{
    (void)dt;

    auto& world = GetEngine()->GetWorld();

    if (!m_CameraOverride)
    {
        glm::mat4 proj = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::vec3 camPos = { 0.0f, 0.0f, 5.0f };
        glm::vec3 camForward = { 0.0f, 0.0f, -1.0f };

        auto camView = world.View<TransformComponent, CameraComponent>();
        for (auto [enttHandle, transform, camComp] : camView.each())
        {
            if (!camComp.Primary || !camComp.Source) continue;

            auto& cam = *camComp.Source;
            cam.SetPosition(transform.Position);
            cam.SetRotation(transform.Rotation);

            proj       = cam.GetProjection();
            view       = cam.GetView();
            camPos     = cam.GetPosition();
            camForward = cam.GetForward();
            break;
        }

        m_View.ViewProjection = proj * view;
        m_View.Projection     = proj;
        m_View.View           = view;
        m_View.Position       = camPos;
        m_View.Forward        = camForward;
        m_View.Width          = m_ViewportWidth;
        m_View.Height         = m_ViewportHeight;
    }

    m_Packet.Entities.clear();
    m_Packet.Lights.clear();

    auto view2 = world.View<TransformComponent, MeshComponent>();

    for (auto [enttHandle, transform, meshComp] : view2.each())
    {
        if (!meshComp.Source) continue;

        EntitySnapshot snap;
        snap.Transform  = transform.GetTransform();
        snap.MeshSource = meshComp.Source;

        Entity entity(enttHandle, &world);
        if (entity.HasComponent<MaterialComponent>())
            snap.MaterialSource = entity.GetComponent<MaterialComponent>().Source;

        m_Packet.Entities.push_back(snap);
    }

    auto lightView = world.View<TransformComponent, LightComponent>();
    for (auto [enttHandle, transform, lightComp] : lightView.each())
    {
        LightData ld;
        ld.Position = transform.Position;

        glm::mat4 rot = glm::toMat4(transform.Rotation);
        ld.Direction = glm::vec3(rot * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));

        ld.Color     = lightComp.Color;
        ld.Intensity = lightComp.Intensity;
        ld.Type      = lightComp.Type;
        m_Packet.Lights.push_back(ld);
    }
}

void RenderSystem::OnEvent(Event& event)
{
    (void)event;
}

} // namespace AF
