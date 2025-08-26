#include "afpch.h"
#include "AF/Core/Window.h"

#ifdef AF_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

namespace AF
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef AF_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		AF_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}