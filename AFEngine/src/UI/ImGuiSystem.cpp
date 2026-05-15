#include "UI/ImGuiSystem.h"

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

    {
        std::filesystem::path iniPath = std::filesystem::current_path() / "imgui.ini";
        static std::string s_IniPath = iniPath.string();
        io.IniFilename = s_IniPath.c_str();
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
}

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace AF
