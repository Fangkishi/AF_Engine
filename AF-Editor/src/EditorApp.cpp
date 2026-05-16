#include <AF.h>

#include "EditorSystem.h"


class EditorApp : public AF::Application
{
public:
    EditorApp()
        : Application("AF-Editor")
    {
    }

    void OnSetup(AF::Engine& engine) override
    {
        AF_LOG_INFO("AF-Editor OnSetup");

        auto& world = engine.GetWorld();

        auto left = world.CreateEntity("Triangle");
        left.AddComponent<AF::MeshComponent>(AF::MeshFactory::CreateTriangle());

        auto light = world.CreateEntity("Directional Light");
        light.GetComponent<AF::TransformComponent>().Position = glm::vec3(0.0f, 3.0f, 5.0f);
        AF::LightComponent lc;
        lc.Color     = { 1.0f, 0.95f, 0.8f };
        lc.Intensity = 2.0f;
        lc.Type      = 0;
        light.AddComponent<AF::LightComponent>(lc);

        engine.AddSystem<AF::EditorSystem>();
        engine.AddSystem<AF::RenderSystem>();
        engine.AddSystem<AF::DeferredRenderPipeline>();
    }
};

AF::Application* CreateApplication()
{
    return new EditorApp();
}
