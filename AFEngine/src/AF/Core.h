#pragma once

#ifdef AF_PLATFORM_WINDOWS
	#ifdef AF_BUILD_DLL
		#define AF_API __declspec(dllexport)
	#else
		#define AF_API __declspec(dllimport)
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