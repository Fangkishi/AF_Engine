#pragma once

// Input —— 静态输入轮询
//
// Engine 在构造时调 Input::SetNativeWindow() 传入窗口句柄。
// 之后引擎任意位置可直接调用 IsKeyPressed / IsMouseButtonPressed 进行状态查询。

#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"

namespace AF {

class Input
{
public:
    static void SetNativeWindow(void* window);

    static bool IsKeyPressed(KeyCode key);
    static bool IsMouseButtonPressed(MouseCode button);

    static float GetMouseX();
    static float GetMouseY();

private:
    static void* s_NativeWindow;
};

} // namespace AF
