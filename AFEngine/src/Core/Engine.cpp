#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Log.h"

#include "Events/Event.h"
#include "Events/WindowEvent.h"

namespace AF {

Engine::Engine(const Config& config)
    : m_Config(config)
{
    AF_LOG_INFO("Initializing Engine: {}", config.Name);

    Window::Desc wd;
    wd.Title = config.Name;
    wd.Width = config.WindowWidth;
    wd.Height = config.WindowHeight;
    wd.VSync = config.VSync;
    m_Window = Window::Create(wd);

    m_World = std::make_unique<World>();
    m_RHIDevice = RHI::RHIDevice::Create();

    m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
    Input::SetNativeWindow(m_Window->GetNativeHandle());

    m_LastFrameTime = m_Timer.ElapsedSeconds();
}

Engine::~Engine()
{
    Shutdown();
}

void Engine::Run()
{
    while (m_Running)
    {
        float now = m_Timer.ElapsedSeconds();
        m_DeltaTime = now - m_LastFrameTime;
        m_LastFrameTime = now;

        m_Window->PollEvents();

        m_RHIDevice->SetClearColor({ 0.1f, 0.1f, 0.15f, 1.0f });
        m_RHIDevice->Clear();

        Update(m_DeltaTime);
        Render();
    }
}

void Engine::Quit()
{
    m_Running = false;
}

void Engine::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent&)
    {
        m_Running = false;
        return true;
    });

    for (auto* system : m_SystemOrder)
    {
        system->OnEvent(event);
        if (event.Handled) break;
    }
}

void Engine::Update(float dt)
{
    for (auto* system : m_SystemOrder)
    {
        system->OnUpdate(dt);
    }
}

void Engine::Render()
{
    m_Window->SwapBuffers();
}

void Engine::Shutdown()
{
    for (auto it = m_SystemOrder.rbegin(); it != m_SystemOrder.rend(); ++it)
    {
        (*it)->OnShutdown();
    }
    m_SystemOrder.clear();
    m_Systems.clear();
    m_RHIDevice.reset();
    m_World.reset();
    m_Window.reset();
}

} // namespace AF
