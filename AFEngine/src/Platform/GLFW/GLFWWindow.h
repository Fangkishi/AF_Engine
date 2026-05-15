#pragma once

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
