# AGENTS.md — AFEngine

## 构建

```powershell
cmake -B build/vs -S . -G "Visual Studio 17 2022" -A x64
cmake --build build/vs --config Debug [--target AFEngine|Sandbox|AF-Editor]
cmake --install build/vs --config Debug --prefix build/install/Debug --component Runtime
cmake --install build/vs --config Release --prefix build/install/Release --component Runtime
```

Release: s/Debug/Release/g。

## CMake 陷阱

- `GLOB_RECURSE` 收集 `src/*.cpp` — 新增文件后必须 reconfigure。
- AFEngine 是 **STATIC** 库；AF-Editor 和 Sandbox 是 **EXE**，链接 AFEngine。
- Assimp 是**预编译 .lib**: `vendor/Assimp/lib/assimp-vc143-mtd.lib` + `zlibstaticd.lib`（仅 Win32），不通过 find_package。
- `ImGuizmo` 无独立 CMake 目标，通过 GLOB_RECURSE 直接包含单 `.cpp`。
- 根 CMakeLists.txt subdir 顺序: vendor(GLFW/Glad/imgui/yaml-cpp/Box2D) → AFEngine → AF-Editor → Sandbox。
- AF-Editor POST_BUILD 自动复制 `assets/` 和 `Resources/` 到 exe 输出目录。
- 无 CI (无 `.github/`)，无 pre-commit hook。

## 架构

```
EntryPoint.h (main — 只能被一个 .cpp include)
  → Application (薄壳)
    → m_Engine.Run()
        PollEvents → Clear → Update(Systems 正序) → OnEvent(正序, Handled 终止) → SwapBuffers
```

- **没有** LayerStack — 全部是 `System`（OnInitialize/OnUpdate/OnEvent/OnShutdown）。
- `System` 注册顺序决定执行顺序。**Editor**: EditorSystem → RenderSystem → DeferredRenderPipeline → ForwardRenderPipeline。**Sandbox**: RenderSystem → DeferredRenderPipeline → ForwardRenderPipeline → ImGuiSystem。
- `OnShutdown` 逆序执行。

## Agent 常见陷阱

- **AF.h 只包含 Core/*.h**（含 EntryPoint.h），不是万能头。每个 exe 只能有一个 `.cpp` include 它（否则 main 多重定义）。其他 `.cpp` 各自 include 所需头。
- **RenderSystem 在 `Renderer/Renderer.h`**，不是 RenderSystem.h。`ForwardRenderPipeline.*` 直接位于 `Renderer/` 下（非子目录）。
- **Material 是独立目录** `Material/`（Material/MaterialInstance/MaterialFactory/MaterialImporter），不在 Renderer 下。还有 `MaterialGraph/`（节点图→GLSL 编译器）和 `ShaderVariant/`（模板+snippet→完整着色器）。
- **TransformComponent.Rotation = `glm::quat`**（弧度制），不是 vec3 Euler。
- `Scope<T>` = `std::unique_ptr<T>`, `Ref<T>` = `std::shared_ptr<T>`, `Unique<T>` = `std::unique_ptr<T>`（Types.h）。
- 行尾: CRLF + UTF-8 BOM。
- 无自动化测试。验证: 跑 `Sandbox.exe` 和 `AF-Editor.exe`。

## 关键约束

| 约束 | 规则 |
|------|------|
| GLFW include | 仅 `Platform/GLFW/` 和 `UI/` |
| GLAD include | 仅 `RHI/OpenGL/` 和 `UI/ImGuiSystem.cpp` |
| JSON 库 | `nlohmann/json` 单头，path `vendor/nlohmann` |
| 字体加载 | `LoadFont()` 必须在 `ImGui_ImplOpenGL3_Init()` 之前。Init 流程: CreateContext → ConfigFlags → ThemeManager → LoadFont → GLFW/OpenGL Init |
| 事件流 | GLFW callback → AF::Event → Window::EventCallbackFn → Engine::OnEvent → System::OnEvent |
| Input | 构造时 `Input::SetNativeWindow()`，之后 `Input::IsKeyPressed()` |

## 渲染管线

- `RenderSystem`（文件 `Renderer/Renderer.h`）: 每帧收集 ECS Mesh/Camera/Light → `RenderView` + `RenderPacket`。支持 `m_CameraOverride`（编辑器覆盖世界相机）。
- `RenderPipeline`（System 子类）: 子类重写 `OnSetup`(一次)+`OnRender`(每帧组装图)。`OnUpdate` 固定逻辑: 创建 RHICommandBuffer → OnRender → m_Graph.Execute → 回放。
- 视口变化: EditorSystem 检测 ViewportPanel 尺寸 → `RenderSystem::SetViewport(w,h)` → `pipeline.Invalidate()` → `m_Graph.Invalidate()` → 下次 Execute 自动 recompile。

## Material 系统流水线（非直观）

```
MaterialGraph (节点图) → MaterialCompiler → ShaderSnippet (GLSL 片段)
  → ShaderLibrary::GetOrCreatePipelineVariant(ShaderTemplate + snippet + 纹理槽)
  → 完整着色器
```

## 目录

```
AFEngine/src/
├── Core/          → Application, Engine, System, Window, Input, Timer, UUID, Types, Log
├── Events/        → Event 基类 + 具体事件类型
├── ECS/           → Entity, World, Components (Transform/Mesh/Material/Light/Camera)
├── Platform/GLFW/ → GLFWWindow, GLFWInput
├── RHI/           → 抽象 + OpenGL 实现 + CommandBuffer + UniformBuffer
├── RenderGraph/   → Node, Graph, Context, PassDesc, PSOCache
├── Renderer/      → RenderSystem(class), Camera, Mesh, RenderPipeline 基类
│   ├── Deferred/  → DeferredRenderPipeline
│   ├── Brdf/      → BRDF 基类 + DefaultLit/ClearCoat
│   └── ForwardRenderPipeline.*
├── Material/      → Material, MaterialInstance, MaterialFactory, MaterialImporter, MaterialParameterStore
├── MaterialGraph/ → 节点图 + MaterialCompiler(→GLSL)
├── ShaderVariant/ → ShaderLibrary, ShaderTemplate, VariantKey
├── Factory/       → MeshFactory (primitive 网格)
└── UI/            → ImGuiSystem, Theme 系统 (ThemeManager/ThemeSerializer/ImGuiThemeApplier)

AF-Editor/src/
├── EditorApp.cpp  → CreateApplication()
├── EditorCamera   → WASD + 右键
├── EditorSystem   → ImGuiSystem 子类
└── Panels/        → ViewportPanel + HierarchyPanel + InspectorPanel
```

## Theme

```
ImGuiSystem::OnInitialize
  → ThemeManager::Initialize("Resources/Themes/", ImGuiThemeApplier)
  → ApplyTheme("Dark")  →  ApplyColors/Style → LoadFont
```
- 主题: `AF-Editor/Resources/Themes/dark.json`。字体: `Resources/Fonts/opensans/OpenSans-{Regular,Bold}.ttf`。缺失自动降级。

## clangd

- `.clangd` 用 `QueryDriver` 自动获取 MSVC include 路径。
- 修改后需重启 clangd。
