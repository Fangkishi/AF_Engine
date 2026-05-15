#pragma once

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
