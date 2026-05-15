#pragma once

namespace AF {

class Event;
class Engine;

class System
{
public:
    virtual ~System() = default;

    virtual void OnInitialize(Engine&) {}
    virtual void OnUpdate(float dt) {}
    virtual void OnEvent(Event&) {}
    virtual void OnShutdown() {}

protected:
    Engine* GetEngine() { return m_Engine; }
    const Engine* GetEngine() const { return m_Engine; }

private:
    Engine* m_Engine = nullptr;
    friend class Engine;
};

} // namespace AF
