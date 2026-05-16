# AGENTS.md — 项目关键上下文

## 构建命令

```powershell
# 配置 + 构建
cmake -B build/vs -S . -G "Visual Studio 17 2022" -A x64
cmake --build build/vs --config Debug

# 仅构建核心库
cmake --build build/vs --target AFEngine --config Debug

# 仅构建 Sandbox
cmake --build build/vs --target Sandbox --config Debug

# 生成干净的分发目录 (仅含 exe + assets + Resources)
cmake --install build/vs --config Debug --prefix build/install/Debug --component Runtime

# Release 分发
cmake --build build/vs --config Release
cmake --install build/vs --config Release --prefix build/install/Release --component Runtime
```

- CMake 使用 `GLOB_RECURSE` 收集 `src/*.cpp`，新增文件后必须 reconfigure。
- Sandbox 和 AF-Editor 均构建。两个可执行文件：`Sandbox.exe`、`AF-Editor.exe`。
- **分发目录** `build/install/<Config>/` 是可直接运行的干净输出，不含 CMake 中间文件。

## 架构

```
EntryPoint.h (main — 只能被一个 .cpp include，否则多重定义)
  → Application (薄壳)
    → Engine
        ├── Window (GLFWWindow)
        ├── System 列表 (顺序执行：OnInitialize → per-frame OnUpdate → OnEvent → OnShutdown)
        ├── Input (静态轮询 Input::IsKeyPressed/IsMouseButtonPressed)
        └── Event Pipeline (GLFW callback → Event → System::OnEvent)
```

**核心设计：**
- **没有** Layer/LayerStack — 全部用 `System`（`OnInitialize/OnUpdate/OnEvent/OnShutdown`）
- **没有** 全局静态渲染器 — 渲染是 System 子类
- **没有** PCH — `afpch.h` 已删除
- `RenderPipeline` 是 System 子类，子类重写 `OnSetup`(一次) + `OnRender`(每帧动态组装图)
- 渲染图内置在 `RenderPipeline::m_Graph`(public)，Auto-Compile(脏标记)，Idempotent AddNode/CreateTexture

## 目录结构

```
AFEngine/src/
├── AF.h                    ← 公开头文件集合
├── Core/                   ← 基础层 (Types, Log, Engine, System, Window, Input, Timer)
├── Events/                 ← 事件系统
├── ECS/                    ← Entity, World, Components (Transform/Mesh/Material/Light/Camera)
├── Platform/GLFW/          ← GLFW 后端 (GLFWWindow, GLFWInput)
├── RHI/                    ← 渲染硬件接口 + OpenGL 实现 + CommandBuffer + UniformBuffer
├── RenderGraph/            ← 渲染图 (Node, Graph, Context, PassDesc)
├── Renderer/               ← RenderSystem, Camera, Mesh, Material, RenderPipeline
├── Renderer/Deferred/      ← DeferredRenderPipeline (内置延迟渲染管线)
├── Renderer/RenderPacket.h ← EntitySnapshot + LightData
├── UI/                     ← ImGuiSystem, Theme 系统, ThemeManager
│   ├── ImGuiSystem.h/.cpp  ← ImGui 上下文、字体、渲染循环
│   ├── Theme.h/.cpp        ← 主题数据结构 + AbstractThemeApplier 虚接口
│   ├── ThemeSerializer.*   ← JSON ↔ Theme 序列化（nlohmann/json）
│   ├── ThemeManager.h/.cpp ← 单例，扫描 Resources/Themes/，切换主题
│   └── ImGuiThemeApplier.* ← 生产实现，直接写 ImGuiStyle
├── oldsrc/                 ← 旧代码，禁止修改或引用

AF-Editor/src/              ← 编辑器（已构建，System-based）
├── EditorApp.cpp           ← Application 入口
├── EditorSystem.h/.cpp     ← ImGuiSystem 子类，编排器
├── EditorCamera.h/.cpp     ← 自由相机 (WASD + 右键旋转)
└── Panels/                 ← Panel 多态面板 (Viewport, Hierarchy)
```

## 关键约束

| 约束 | 说明 |
|------|------|
| **GLFW include** | `#include <GLFW/glfw3.h>` 仅 `Platform/GLFW/` 和 `UI/` |
| **GLAD include** | 仅 `RHI/OpenGL/` 和 `UI/ImGuiSystem.cpp` |
| **行尾** | CRLF + UTF-8 BOM |
| **无 PCH** | 每个 `.cpp` 精确 include 所需头文件 |
| **EntryPoint** | `main()` 定义在 `Core/EntryPoint.h`。每个可执行文件只能有一个 `.cpp` include `<AF.h>`（因其包含 EntryPoint.h）。其他 `.cpp` 各自 include 所需头文件 |
| **事件流** | GLFW callback → `AF::Event` → `Window::EventCallbackFn` → `Engine::OnEvent` → `System::OnEvent` |
| **Input** | Engine 构造时调 `Input::SetNativeWindow()`，之后任意处可 `Input::IsKeyPressed()` |
| **字体加载顺序** | `ImGuiThemeApplier::LoadFont()` 必须在 `ImGui_ImplOpenGL3_Init()` 之前调用（字体纹理需在 backend init 前构建）。当前 Init 流程: `CreateContext → ConfigFlags → ThemeManager → LoadFont → GLFW/OpenGL Init` |
| **JSON 库** | `nlohmann/json` 3.12 单头文件，位于 `vendor/nlohmann/nlohmann/json.hpp`，include path 为 `vendor/nlohmann` |

## Theme 系统

```
ImGuiSystem::OnInitialize()
  → ThemeManager::Initialize("Resources/Themes/", ImGuiThemeApplier)
  → ThemeManager::ApplyTheme("Dark")        // 从 dark.json 加载
  → ImGuiThemeApplier::ApplyColors/Style    // 写 ImGuiStyle
  → ImGuiThemeApplier::LoadFont             // OpenSans Regular + Bold
```

- 主题配置文件: `AF-Editor/Resources/Themes/dark.json`
- 字体: `AF-Editor/Resources/Fonts/opensans/OpenSans-{Regular,Bold}.ttf`
- 文件缺失时自动降级到内置默认主题和 ImGui 默认字体
- `AbstractThemeApplier` 虚接口支持 Mock 测试
- `ThemeSerializer::s_ColorMap` 将 JSON 键名映射到 `ImGuiCol_` 枚举索引

## 核心约定

- **System 注册顺序决定 OnUpdate 顺序**：`AddSystem` 先的 OnUpdate 先执行
- **渲染管线顺序**：`EditorSystem`/ImGui → `RenderSystem` → `DeferredRenderPipeline`（ImGui 提前更新相机，渲染系统读取，管线执行）
- **TransformComponent.Rotation = `glm::quat`**，弧度制。不是 vec3 Euler
- **Camera**：`SetRotation(quat)` 直接传四元数，`RecalculateView` 用 `glm::toMat4(quat)` 算 front
- **EditorCamera**：`GetRotation()` 返回 `qYaw * qPitch`，yaw 用 `angleAxis(-(yawRad+π/2), Y)` 对齐 OpenGL -Z forward 约定
- **RenderGraph**：`Execute` 入口自动 `if (!m_Compiled) Compile()`；`ClearResources` 含纹理重置
- **命令队列**：`RenderPipeline::OnUpdate` 创建 `RHICommandBuffer`，`OnRender` 录 UBO，`Execute` 录绘制，帧末统一回放
- **视口尺寸**：由 EditorSystem 检测 ViewportPanel 尺寸变化 → `RenderSystem::SetViewport(w,h)` → pipeline.Invalidate
- **纹理获取**：`pipeline.GetOutput("finalComposite")` → `m_Graph.GetResourceTexture(name)` → `Ref<RHITexture2D>`
- **层级面板**：`HierarchyPanel` 显示所有实体。场景相机作为实体可见。编辑器相机不再是实体（由 `EditorCamera` 直接持有 `Camera` 对象），因此不会出现在层级中
- **相机切换**：`ViewportPanel` 顶部下拉框选择活动相机。`EditorSystem` 管理 `m_ActiveCameraUUID`（null=编辑相机）。每帧 `OnUpdate` 构建 `RenderView` 推送至 `RenderSystem::SetCameraView()`
- **RenderSystem 双路径**：`m_CameraOverride` 为 true 时跳过世界相机遍历（编辑器模式），为 false 时回退原有逻辑（Sandbox 兼容）
- **右键菜单互斥**：实体右键菜单打开后通过 `anyEntityMenu` 标志阻止空白区弹窗同时弹出
- `Ref<T>` = `std::shared_ptr<T>`, `Unique<T>` = `std::unique_ptr<T>`

## clangd

- `.clangd` 使用 `QueryDriver` 自动获取 MSVC/SDK include 路径
- 修改 `.clangd` 后需重启 clangd 语言服务器

## 测试

无自动化测试。验证方式：运行 `Sandbox.exe` 和 `AF-Editor.exe`。
