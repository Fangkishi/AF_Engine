#pragma once

namespace AF {

class Event;

class Panel
{
public:
    virtual ~Panel() = default;

    virtual const char* GetName() const = 0;
    virtual void OnImGuiRender() {}
    virtual void OnUpdate(float dt) { (void)dt; }
    virtual void OnEvent(Event&) {}
};

} // namespace AF
