#pragma once

#include "Core/Types.h"
#include "Core/System.h"
#include "Core/Window.h"
#include "Core/Timer.h"
#include "ECS/World.h"
#include "RHI/RHIDevice.h"

#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace AF {

class Engine : public NonCopyable
{
public:
    struct Config
    {
        std::string Name = "AFEngine";
        uint32_t WindowWidth = 1920;
        uint32_t WindowHeight = 1080;
        bool VSync = true;
    };

    explicit Engine(const Config& config);
    ~Engine();

    void Run();
    void Quit();

    void OnEvent(Event& event);

    template <typename T, typename... Args>
    T& AddSystem(Args&&... args);

    template <typename T>
    T& GetSystem();

    Window& GetWindow() { return *m_Window; }
    World& GetWorld() { return *m_World; }
    RHI::RHIDevice& GetDevice() { return *m_RHIDevice; }
    float GetDeltaTime() const { return m_DeltaTime; }

private:
    void Update(float dt);
    void Render();
    void Shutdown();

    Config m_Config;
    Unique<Window> m_Window;
    Unique<World> m_World;
    Unique<RHI::RHIDevice> m_RHIDevice;
    std::vector<System*> m_SystemOrder;
    std::unordered_map<std::type_index, Unique<System>> m_Systems;
    Timer m_Timer;
    float m_DeltaTime = 0.016f;
    float m_LastFrameTime = 0.0f;
    bool m_Running = true;
};

template <typename T, typename... Args>
T& Engine::AddSystem(Args&&... args)
{
    auto sys = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = sys.get();
    m_Systems[std::type_index(typeid(T))] = std::move(sys);
    m_SystemOrder.push_back(ptr);
    ptr->m_Engine = this;
    ptr->OnInitialize(*this);
    return *ptr;
}

template <typename T>
T& Engine::GetSystem()
{
    auto it = m_Systems.find(std::type_index(typeid(T)));
    AF_CORE_ASSERT(it != m_Systems.end(), "System not registered");
    return *static_cast<T*>(it->second.get());
}

} // namespace AF
