#include <AF.h>
#include <imgui.h>


class Sandbox : public AF::Application
{
public:
    Sandbox()
        : Application("Sandbox")
    {
    }

    void OnSetup(AF::Engine& engine) override
    {
        AF_LOG_INFO("Sandbox OnSetup");

        auto& world = engine.GetWorld();

        auto left = world.CreateEntity("Left Triangle");
        left.AddComponent<AF::MeshComponent>(AF::Mesh::CreateTriangle());
        left.GetComponent<AF::TransformComponent>().Position.x = -1.5f;

        auto mid = world.CreateEntity("Mid Triangle");
        mid.AddComponent<AF::MeshComponent>(AF::Mesh::CreateTriangle());
        auto& midT = mid.GetComponent<AF::TransformComponent>();
        midT.Rotation = glm::angleAxis(0.5f, glm::vec3(0, 0, 1));
        midT.Scale = glm::vec3(0.7f);

        auto right = world.CreateEntity("Right Triangle");
        right.AddComponent<AF::MeshComponent>(AF::Mesh::CreateTriangle());
        auto& rightT = right.GetComponent<AF::TransformComponent>();
        rightT.Position.x = 1.5f;
        rightT.Scale = glm::vec3(1.5f, 0.6f, 1.0f);

        auto camera = world.CreateEntity("Main Camera");
        camera.GetComponent<AF::TransformComponent>().Position = { 0.0f, 0.0f, 5.0f };
        auto cam = std::make_shared<AF::Camera>();
        cam->SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);
        cam->SetPosition({ 0.0f, 0.0f, 5.0f });
        AF::CameraComponent cc;
        cc.Source  = cam;
        cc.Primary = true;
        camera.AddComponent<AF::CameraComponent>(cc);

        auto light = world.CreateEntity("Directional Light");
        light.GetComponent<AF::TransformComponent>().Position = glm::vec3(0.0f, 3.0f, 5.0f);
        light.GetComponent<AF::TransformComponent>().Rotation = glm::quat(glm::vec3(glm::radians(30.0f), glm::radians(-45.0f), 0.0f));
        AF::LightComponent lc;
        lc.Color     = { 1.0f, 0.95f, 0.8f };
        lc.Intensity = 2.0f;
        lc.Type      = 0;
        light.AddComponent<AF::LightComponent>(lc);

        engine.AddSystem<AF::RenderSystem>();
        engine.AddSystem<AF::DeferredRenderPipeline>();

        auto& ui = engine.AddSystem<AF::ImGuiSystem>();
        ui.SetOnImGui([]() {
            ImGui::Begin("AFEngine");
            ImGui::Text("Deferred Rendering Pipeline");
            ImGui::Separator();

            auto& io = ImGui::GetIO();
            ImGui::Text("FPS: %.1f (%.2f ms)", io.Framerate, io.DeltaTime * 1000.0f);

            if (ImGui::CollapsingHeader("Camera"))
            {
                ImGui::Text("Mode: Perspective (FOV 60)");
                ImGui::Text("Position: (0, 0, 5)");
            }

            if (ImGui::CollapsingHeader("Scene"))
            {
                ImGui::Text("Entities: 5 (3 triangles + camera + light)");
                ImGui::Text("Directional Light: intensity=2.0");
            }

            if (ImGui::CollapsingHeader("GBuffer"))
            {
                ImGui::Text("RT0: gAlbedo    (RGBA8)");
                ImGui::Text("RT1: gNormal    (RGBA16F)");
                ImGui::Text("RT2: gMaterial  (RGBA8)");
                ImGui::Text("DS:  gDepth     (Depth32)");
            }

            ImGui::End();
        });
    }
};

AF::Application* CreateApplication()
{
    return new Sandbox();
}
