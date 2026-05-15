#include "Core/Application.h"

namespace AF {

Application::Application(std::string_view name)
    : m_Engine(Engine::Config{std::string(name)})
{
}

Application::~Application() = default;

void Application::Run()
{
    OnSetup(m_Engine);
    m_Engine.Run();
}

} // namespace AF
