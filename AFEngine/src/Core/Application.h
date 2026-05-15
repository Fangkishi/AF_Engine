#pragma once

#include "Core/Types.h"
#include "Core/Engine.h"

#include <string>

namespace AF {

class Application : public NonCopyable
{
public:
    explicit Application(std::string_view name);
    virtual ~Application();

    virtual void OnSetup(Engine& engine) {}

    void Run();

    Engine& GetEngine() { return m_Engine; }

private:
    Engine m_Engine;
};

} // namespace AF
