#include "Core/Window.h"
#include "Core/Assert.h"

#ifdef AF_PLATFORM_WINDOWS
    #include "Platform/GLFW/GLFWWindow.h"
#endif

namespace AF {

Unique<Window> Window::Create(const Desc& desc)
{
#ifdef AF_PLATFORM_WINDOWS
    return std::make_unique<GLFWWindow>(desc);
#else
    AF_CORE_ASSERT(false, "Unsupported platform!");
    return nullptr;
#endif
}

} // namespace AF
