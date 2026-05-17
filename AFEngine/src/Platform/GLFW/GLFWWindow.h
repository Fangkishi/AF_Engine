#pragma once

// GLFWWindow —— GLFW 后端窗口实现
//
// 实现 Window 抽象接口，封装 GLFW 窗口生命周期：
// - 构造：初始化 GLFW → 创建窗口 + OpenGL context → 设置回调
// - 析构：销毁窗口 → 引用计数归零时 glfwTerminate
// - 回调链：GLFW callback → Event 封装 → EventCallbackFn

#include "Core/Window.h"

struct GLFWwindow;

namespace AF {

class GLFWWindow : public Window
{
public:
    explicit GLFWWindow(const Desc& desc);
    ~GLFWWindow() override;

    void PollEvents() override;
    void SwapBuffers() override;

    void SetEventCallback(const EventCallbackFn& callback) override { m_EventCallback = callback; }

    void* GetNativeHandle() const override;
    uint32_t GetWidth() const override { return m_Data.Width; }
    uint32_t GetHeight() const override { return m_Data.Height; }
    void SetVSync(bool enabled) override;
    bool IsVSync() const override { return m_Data.VSync; }

private:
    GLFWwindow* m_Handle = nullptr;
    EventCallbackFn m_EventCallback;

    struct
    {
        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool VSync = true;
    } m_Data;
};

} // namespace AF
