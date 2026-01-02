project(AF-Editor)

file(GLOB_RECURSE EDITOR_SOURCES 
    "src/*.cpp"
    "src/*.h"
)

add_executable(AF-Editor ${EDITOR_SOURCES})

# 包含路径
target_include_directories(AF-Editor PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/spdlog/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/entt/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/ImGuizmo"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/Assimp/include"
)

# 链接库
target_link_libraries(AF-Editor PRIVATE AFEngine)

# 设置运行时工作目录为项目根目录 (针对 Visual Studio)
set_target_properties(AF-Editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/..")

# 自动同步 assets 和 Resources 文件夹到构建目录
add_custom_target(AF-Editor-Resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
    "${CMAKE_CURRENT_BINARY_DIR}/Resources"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini"
    "${CMAKE_CURRENT_BINARY_DIR}/imgui.ini"
    COMMENT "Syncing assets, Resources and imgui.ini to build directory..."
    VERBATIM
)

# 让 AF-Editor 依赖于资源同步目标
add_dependencies(AF-Editor AF-Editor-Resources)

# 宏定义
target_compile_definitions(AF-Editor PRIVATE 
    YAML_CPP_STATIC_DEFINE
)

if(WIN32)
    # 设置为控制台程序 (对应 premake 的 kind "ConsoleApp")
    set_target_properties(AF-Editor PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()
