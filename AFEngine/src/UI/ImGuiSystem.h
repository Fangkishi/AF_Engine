#pragma once

// ImGuiSystem —— ImGui UI 系统
//
// 封装 Dear ImGui 的上下文初始化和渲染循环。
// 子类可重写 OnImGui()，或通过 SetOnImGui 设置回调函数。
// 支持多视口（ViewportsEnable）—— 允许 ImGui 窗口拖出主窗口。

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
    /// 子类可重写此方法实现自定义 ImGui 内容
    virtual void OnImGui() {}

private:
    OnImGuiFn m_OnImGuiFn;
};

} // namespace AF
