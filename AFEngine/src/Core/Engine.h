#pragma once

// Engine —— 引擎核心编排器
//
// 生命周期：构造（Window/World/RHIDevice）→ AddSystem → Run → Shutdown
// 主循环：PollEvents → Clear → Update(systems) → SwapBuffers
//
// System 按 AddSystem 注册顺序执行 OnUpdate，逆序执行 OnShutdown。

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

    /// 启动主循环，直到调用 Quit()
    void Run();
    void Quit();

    void OnEvent(Event& event);

    /// 注册 System（类型 T 须继承 System），返回引用
    template <typename T, typename... Args>
    T& AddSystem(Args&&... args);

    /// 按类型 T 获取已注册的 System 引用
    template <typename T>
    T& GetSystem();

    Window& GetWindow() { return *m_Window; }
    World& GetWorld() { return *m_World; }
    RHI::RHIDevice& GetDevice() { return *m_RHIDevice; }
    float GetDeltaTime() const { return m_DeltaTime; }
    float GetElapsedTime() const { return m_LastFrameTime; }

private:
    void Update(float dt);
    void Render();
    void Shutdown();

    Config m_Config;
    Unique<Window> m_Window;
    Unique<World> m_World;
    Unique<RHI::RHIDevice> m_RHIDevice;
    std::vector<System*> m_SystemOrder;                // 按注册顺序排列
    std::unordered_map<std::type_index, Unique<System>> m_Systems;  // 类型 → 实例
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
