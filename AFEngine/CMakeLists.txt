project(AFEngine)

# 递归寻找源文件
file(GLOB_RECURSE AF_SOURCES 
    "src/*.cpp"
    "src/*.h"
    "vendor/stb_image/*.cpp"
    "vendor/stb_image/*.h"
    "vendor/glm/glm/*.hpp"
    "vendor/glm/glm/*.inl"
    "vendor/ImGuizmo/ImGuizmo.cpp"
    "vendor/ImGuizmo/ImGuizmo.h"
)

add_library(AFEngine STATIC ${AF_SOURCES})

# 包含路径
target_include_directories(AFEngine PUBLIC 
    "src"
    "vendor/spdlog/include"
    "vendor/GLFW/include"
    "vendor/Glad/include"
    "vendor/imgui"
    "vendor/glm"
    "vendor/stb_image"
    "vendor/entt/include"
    "vendor/yaml-cpp/include"
    "vendor/ImGuizmo"
    "vendor/Box2D/include"
    "vendor/Assimp/include"
)

# 宏定义
target_compile_definitions(AFEngine PUBLIC 
    _CRT_SECURE_NO_WARNINGS
    GLFW_INCLUDE_NONE
    YAML_CPP_STATIC_DEFINE
)

# 链接库
find_package(OpenGL REQUIRED)
target_link_libraries(AFEngine PUBLIC 
    glfw
    Glad
    ImGui
    yaml-cpp
    box2d
    OpenGL::GL
)

# Windows 特定配置 (Assimp 库)
if(WIN32)
    target_link_libraries(AFEngine PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/Assimp/lib/assimp-vc143-mtd.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/Assimp/lib/zlibstaticd.lib"
    )
endif()

# 预编译头配置
if(MSVC)
    target_precompile_headers(AFEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/afpch.h")
endif()

# 针对 ImGuizmo 禁用 PCH (对应 premake 的 flags { "NoPCH" })
set_source_files_properties("vendor/ImGuizmo/ImGuizmo.cpp" PROPERTIES SKIP_PRECOMPILED_HEADER ON)
