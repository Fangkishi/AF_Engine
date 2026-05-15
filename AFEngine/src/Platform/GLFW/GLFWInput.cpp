#include "Core/Input.h"

#include <GLFW/glfw3.h>

namespace AF {

void* Input::s_NativeWindow = nullptr;

void Input::SetNativeWindow(void* window)
{
    s_NativeWindow = window;
}

bool Input::IsKeyPressed(KeyCode key)
{
    auto* window = static_cast<GLFWwindow*>(s_NativeWindow);
    return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
}

bool Input::IsMouseButtonPressed(MouseCode button)
{
    auto* window = static_cast<GLFWwindow*>(s_NativeWindow);
    return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
}

float Input::GetMouseX()
{
    auto* window = static_cast<GLFWwindow*>(s_NativeWindow);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return static_cast<float>(x);
}

float Input::GetMouseY()
{
    auto* window = static_cast<GLFWwindow*>(s_NativeWindow);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return static_cast<float>(y);
}

} // namespace AF
