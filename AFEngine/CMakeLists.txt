project(AFEngine)

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

target_compile_definitions(AFEngine PUBLIC 
    $<$<CONFIG:Debug>:AF_DEBUG>
    $<$<CONFIG:Release>:AF_RELEASE>
    _CRT_SECURE_NO_WARNINGS
    GLFW_INCLUDE_NONE
    YAML_CPP_STATIC_DEFINE
)

find_package(OpenGL REQUIRED)
target_link_libraries(AFEngine PUBLIC 
    glfw
    Glad
    ImGui
    yaml-cpp
    box2d
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(AFEngine PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/Assimp/lib/assimp-vc143-mtd.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/Assimp/lib/zlibstaticd.lib"
    )
endif()
