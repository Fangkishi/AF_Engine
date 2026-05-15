#pragma once

#include "Core/System.h"

#include <functional>

namespace AF {

class ImGuiSystem : public System
{
public:
    void OnInitialize(Engine& engine) override;
    void OnUpdate(float dt) override;
    void OnEvent(Event& event) override;
    void OnShutdown() override final;

    using OnImGuiFn = std::function<void()>;
    void SetOnImGui(OnImGuiFn fn) { m_OnImGuiFn = std::move(fn); }

protected:
    virtual void OnImGui() {}

private:
    OnImGuiFn m_OnImGuiFn;
};

} // namespace AF
