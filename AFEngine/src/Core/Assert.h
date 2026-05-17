#pragma once

// 断言宏 —— Debug 模式下检查条件，失败时打印日志并触发断点

#include "Core/Platform.h"
#include "Core/Log.h"

#include <filesystem>

#ifdef AF_DEBUG

    #ifdef AF_PLATFORM_WINDOWS
        #define AF_DEBUGBREAK() __debugbreak()
    #else
        #include <csignal>
        #define AF_DEBUGBREAK() std::raise(SIGTRAP)
    #endif

    #define AF_ASSERT_IMPL(check, msg) \
        do { \
            if (!(check)) { \
                AF_LOG_ERROR("{}", msg); \
                AF_DEBUGBREAK(); \
            } \
        } while (false)

    #define AF_ASSERT_NO_MSG(check) \
        AF_ASSERT_IMPL(check, \
            fmt::format("Assertion '{}' failed at {}:{}", \
                #check, \
                std::filesystem::path(__FILE__).filename().string(), \
                __LINE__))

    #define AF_ASSERT_GET(_1, _2, NAME, ...) NAME
    #define AF_ASSERT(...) \
        AF_ASSERT_GET(__VA_ARGS__, AF_ASSERT_IMPL, AF_ASSERT_NO_MSG)(__VA_ARGS__)
    #define AF_CORE_ASSERT(...) AF_ASSERT(__VA_ARGS__)

#else

    // Release 模式下断言为空操作
    #define AF_ASSERT(...)
    #define AF_CORE_ASSERT(...)

#endif
