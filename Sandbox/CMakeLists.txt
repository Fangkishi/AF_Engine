project(Sandbox)

file(GLOB_RECURSE SANDBOX_SOURCES 
    "src/*.cpp"
    "src/*.h"
)

add_executable(Sandbox ${SANDBOX_SOURCES})

# 包含路径
target_include_directories(Sandbox PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/spdlog/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/entt/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/ImGuizmo"
    "${CMAKE_CURRENT_SOURCE_DIR}/../AFEngine/vendor/yaml-cpp/include"
)

# 链接库
target_link_libraries(Sandbox PRIVATE AFEngine)

# 设置运行时工作目录为项目根目录 (针对 Visual Studio)
set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/..")

# 自动同步 assets 文件夹到构建目录
add_custom_target(Sandbox-Resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMENT "Syncing assets to build directory..."
    VERBATIM
)

# 让 Sandbox 依赖于资源同步目标
add_dependencies(Sandbox Sandbox-Resources)

# 将 assets 复制到 exe 输出目录，确保运行时相对路径可靠
add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:Sandbox>/assets"
    COMMENT "Deploying assets to output directory..."
    VERBATIM
)

# 宏定义
target_compile_definitions(Sandbox PRIVATE 
    YAML_CPP_STATIC_DEFINE
)

if(WIN32)
    # 设置为控制台程序
    set_target_properties(Sandbox PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

# Install — 生成干净的分发目录
install(TARGETS Sandbox RUNTIME DESTINATION . COMPONENT Runtime)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION assets COMPONENT Runtime)
