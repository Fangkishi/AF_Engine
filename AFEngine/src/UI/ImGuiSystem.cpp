#include "UI/ImGuiSystem.h"
#include "UI/ImGuiThemeApplier.h"
#include "UI/ThemeManager.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <filesystem>

#include "Core/Engine.h"
#include "Core/Log.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace AF {

void ImGuiSystem::OnInitialize(Engine& engine)
{
    AF_LOG_INFO("ImGuiSystem: initializing...");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // 将 imgui.ini 固定到工作目录，避免 CWD 不确定导致布局丢失
    {
        std::filesystem::path iniPath = std::filesystem::current_path() / "imgui.ini";
        static std::string s_IniPath = iniPath.string();
        io.IniFilename = s_IniPath.c_str();
    }

    // 初始化主题系统
    auto& tm = ThemeManager::Get();
    tm.Initialize("Resources/Themes/", std::make_unique<ImGuiThemeApplier>());

    if (!tm.ApplyTheme("Dark"))
    {
        Theme builtin = Theme::CreateDefaultDark();
        builtin.Name = "Built-in Dark";
        tm.ApplyTheme(builtin);
    }

    auto* window = static_cast<GLFWwindow*>(engine.GetWindow().GetNativeHandle());
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    AF_LOG_INFO("ImGuiSystem: initialized");
}

void ImGuiSystem::OnUpdate(float dt)
{
    (void)dt;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_OnImGuiFn)
        m_OnImGuiFn();
    else
        OnImGui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 多视口支持：更新和渲染浮动窗口
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}

/// 事件过滤——ImGui 需要优先截获输入事件
void ImGuiSystem::OnEvent(Event& event)
{
    if (event.Handled)
        return;

    ImGuiIO& io = ImGui::GetIO();
    EventDispatcher dispatcher(event);

    dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent&) {
        return io.WantCaptureKeyboard;
    });
    dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent&) {
        return io.WantCaptureKeyboard;
    });
    dispatcher.Dispatch<KeyTypedEvent>([&](KeyTypedEvent&) {
        return io.WantCaptureKeyboard;
    });
    dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent&) {
        return io.WantCaptureMouse;
    });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent&) {
        return io.WantCaptureMouse;
    });
    dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent&) {
        return io.WantCaptureMouse;
    });
    dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent&) {
        return io.WantCaptureMouse;
    });
}

void ImGuiSystem::OnShutdown()
{
    AF_LOG_INFO("ImGuiSystem: shutting down...");
    ThemeManager::Get().Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace AF
