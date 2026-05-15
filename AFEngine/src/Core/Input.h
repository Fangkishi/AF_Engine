#pragma once

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
