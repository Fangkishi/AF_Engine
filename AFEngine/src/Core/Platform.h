#pragma once

// 平台检测宏，根据编译器预定义宏确定目标平台

#ifdef _WIN32
    #ifdef _WIN64
        #define AF_PLATFORM_WINDOWS
    #else
        #error "x86 builds are not supported!"
    #endif
#elif defined(__linux__)
    #define AF_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define AF_PLATFORM_MACOS
#else
    #error "Unsupported platform!"
#endif
