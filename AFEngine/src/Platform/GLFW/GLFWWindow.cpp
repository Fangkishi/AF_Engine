#include "Platform/GLFW/GLFWWindow.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"

namespace AF {

static uint8_t s_GLFWWindowCount = 0;

static void GLFWErrorCallback(int error, const char* description)
{
    AF_LOG_ERROR("GLFW Error ({}): {}", error, description);
}

GLFWWindow::GLFWWindow(const Desc& desc)
{
    m_Data.Width = desc.Width;
    m_Data.Height = desc.Height;

    AF_LOG_INFO("Creating window: {} ({}x{})", desc.Title, desc.Width, desc.Height);

    // 首次创建窗口时全局初始化 GLFW
    if (s_GLFWWindowCount == 0)
    {
        int success = glfwInit();
        AF_CORE_ASSERT(success, "Failed to initialize GLFW!");
        glfwSetErrorCallback(GLFWErrorCallback);
    }

    // OpenGL 4.5 Core Profile 上下文
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef AF_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    m_Handle = glfwCreateWindow(
        static_cast<int>(desc.Width),
        static_cast<int>(desc.Height),
        desc.Title.c_str(),
        nullptr, nullptr);
    AF_CORE_ASSERT(m_Handle, "Failed to create GLFW window!");
    ++s_GLFWWindowCount;

    glfwMakeContextCurrent(m_Handle);

    int gladStatus = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    AF_CORE_ASSERT(gladStatus, "Failed to initialize GLAD!");

    AF_LOG_INFO("OpenGL Info:");
    AF_LOG_INFO("  Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    AF_LOG_INFO("  Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    AF_LOG_INFO("  Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    // GLFW 回调注册 —— 通过窗口 user pointer 路由至 GLFWWindow 实例
    glfwSetWindowUserPointer(m_Handle, this);

    // 窗口关闭
    glfwSetWindowCloseCallback(m_Handle, [](GLFWwindow* window)
    {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self->m_EventCallback)
        {
            WindowCloseEvent event;
            self->m_EventCallback(event);
        }
    });

    // 窗口尺寸变化
    glfwSetWindowSizeCallback(m_Handle, [](GLFWwindow* window, int width, int height)
    {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        self->m_Data.Width = static_cast<uint32_t>(width);
        self->m_Data.Height = static_cast<uint32_t>(height);
        if (self->m_EventCallback)
        {
            WindowResizeEvent event(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            self->m_EventCallback(event);
        }
    });

    // 键盘事件
    glfwSetKeyCallback(m_Handle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        (void)scancode;
        (void)mods;
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (!self->m_EventCallback) return;

        switch (action)
        {
            case GLFW_PRESS:
            {
                KeyPressedEvent event(static_cast<KeyCode>(key), false);
                self->m_EventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(static_cast<KeyCode>(key));
                self->m_EventCallback(event);
                break;
            }
            case GLFW_REPEAT:
            {
                KeyPressedEvent event(static_cast<KeyCode>(key), true);
                self->m_EventCallback(event);
                break;
            }
        }
    });

    // 鼠标按键事件
    glfwSetMouseButtonCallback(m_Handle, [](GLFWwindow* window, int button, int action, int mods)
    {
        (void)mods;
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (!self->m_EventCallback) return;

        switch (action)
        {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                self->m_EventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
                self->m_EventCallback(event);
                break;
            }
        }
    });

    // 滚轮事件
    glfwSetScrollCallback(m_Handle, [](GLFWwindow* window, double xOffset, double yOffset)
    {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (!self->m_EventCallback) return;

        MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
        self->m_EventCallback(event);
    });

    // 鼠标移动事件
    glfwSetCursorPosCallback(m_Handle, [](GLFWwindow* window, double xPos, double yPos)
    {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (!self->m_EventCallback) return;

        MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
        self->m_EventCallback(event);
    });

    SetVSync(desc.VSync);
}

GLFWWindow::~GLFWWindow()
{
    if (m_Handle)
    {
        glfwDestroyWindow(m_Handle);
        m_Handle = nullptr;
    }

    --s_GLFWWindowCount;
    if (s_GLFWWindowCount == 0)
    {
        glfwTerminate();
    }
}

void GLFWWindow::PollEvents()
{
    glfwPollEvents();
}

void GLFWWindow::SwapBuffers()
{
    glfwSwapBuffers(m_Handle);
}

void* GLFWWindow::GetNativeHandle() const
{
    return static_cast<void*>(m_Handle);
}

void GLFWWindow::SetVSync(bool enabled)
{
    m_Data.VSync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

} // namespace AF
