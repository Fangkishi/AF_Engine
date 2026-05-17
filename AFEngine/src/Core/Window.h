#pragma once

// 窗口抽象接口 —— 隐藏平台窗口实现细节

#include "Core/Types.h"

#include <cstdint>
#include <functional>
#include <string>

namespace AF {

class Event;

class Window : public NonCopyable
{
public:
    /// 事件回调类型（由 Engine 注入，事件从 GLFW → Event → Engine → System）
    using EventCallbackFn = std::function<void(Event&)>;

    /// 窗口创建参数
    struct Desc
    {
        std::string Title = "AFEngine";
        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool VSync = true;
    };

    virtual ~Window() = default;

    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;

    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

    virtual void* GetNativeHandle() const = 0;
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;

    /// 工厂：根据平台宏返回对应后端实例
    static Unique<Window> Create(const Desc& desc);
};

} // namespace AF
