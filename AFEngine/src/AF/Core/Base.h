#pragma once

#include "AF/Core/PlatformDetection.h"

#include <memory>

#ifdef AF_DEBUG
#if defined(AF_PLATFORM_WINDOWS)
#define AF_DEBUGBREAK() __debugbreak()
#elif defined(AF_PLATFORM_LINUX)
#include <signal.h>
#define AF_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define AF_ENABLE_ASSERTS
#else
#define AF_DEBUGBREAK()
#endif

#define AF_EXPAND_MACRO(x) x
#define AF_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define AF_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace AF {
	template <typename T>
	using Scope = std::unique_ptr<T>;

	template <typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#include "AF/Core/Log.h"
#include "AF/Core/Assert.h"
