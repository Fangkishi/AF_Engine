#pragma once

#include "AF/Core/Base.h"
#include "AF/Core/Log.h"
#include <filesystem>

#ifdef AF_ENABLE_ASSERTS

	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define AF_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { AF##type##ERROR(msg, __VA_ARGS__); AF_DEBUGBREAK(); } }
	#define AF_INTERNAL_ASSERT_WITH_MSG(type, check, ...) AF_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define AF_INTERNAL_ASSERT_NO_MSG(type, check) AF_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", AF_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)
	
	#define AF_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define AF_INTERNAL_ASSERT_GET_MACRO(...) AF_EXPAND_MACRO( AF_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, AF_INTERNAL_ASSERT_WITH_MSG, AF_INTERNAL_ASSERT_NO_MSG) )
	
	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define AF_ASSERT(...) AF_EXPAND_MACRO( AF_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define AF_CORE_ASSERT(...) AF_EXPAND_MACRO( AF_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define AF_ASSERT(...)
	#define AF_CORE_ASSERT(...)
#endif
