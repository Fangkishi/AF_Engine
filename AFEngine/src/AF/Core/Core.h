#pragma once

#include <memory>

#ifdef AF_PLATFORM_WINDOWS
	#if HZ_DYNAMIC_LINK
		#ifdef AF_BUILD_DLL
			#define AF_API __declspec(dllexport)
		#else
			#define AF_API __declspec(dllimport)
		#endif
	#else
		#define AF_API
	#endif
#else
	#error AFEngine only support Windows !
#endif

#if AF_DEBUG
#define AF_ENABLE_ASSERTS
#endif

#ifdef AF_ENABLE_ASSERTS
	#define AF_ASSERT(x, ...) { if(!(x)) { AF_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define AF_CORE_ASSERT(x, ...) { if(!(x)) { AF_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define AF_ASSERT(x, ...)
	#define AF_CORE_ASSERT(x, ...)
#endif 

#define BIT(x) (1 << x)

#define AF_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace AF {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}