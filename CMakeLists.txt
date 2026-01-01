cmake_minimum_required(VERSION 3.16)
project(AFEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 导出编译命令用于 clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 常用变量
set(AF_VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/AFEngine/vendor)

# --- 依赖库定义 (静态库) ---

# GLFW
add_subdirectory(AFEngine/vendor/GLFW)

# Glad
add_subdirectory(AFEngine/vendor/Glad)

# ImGui
add_subdirectory(AFEngine/vendor/imgui)

# yaml-cpp
add_subdirectory(AFEngine/vendor/yaml-cpp)

# Box2D
add_subdirectory(AFEngine/vendor/Box2D)

# --- 项目定义 ---

# AFEngine 核心库
add_subdirectory(AFEngine)

# AF-Editor 应用程序
add_subdirectory(AF-Editor)
