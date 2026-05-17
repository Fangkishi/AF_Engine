# AFEngine

[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

一个基于 C++17 的游戏引擎练习项目，使用 OpenGL 4.5+ 渲染后端。

## 特性

### 渲染系统

- **渲染图 (Render Graph)** — 有向无环图 (DAG) 管理 Pass 依赖，自动资源调度与生命周期管理，支持动态添加自定义渲染节点
- **延迟渲染管线** — G-Buffer 阶段（Albedo + Normal + Material + Depth）→ 光照阶段（PBR，HDR 曝光控制）
- **前向渲染管线** — 半透明物体 forward pass
- **BRDF 框架** — `DefaultLitBRDF` 和 `ClearCoatBRDF` 可扩展基类
- **性能优化** — 状态追踪减少 VAO/Uniform 切换，按材质和网格排序，纹理缓存去重

### 场景系统

- **基于 EnTT ECS** — Entity/World/Components（Transform/Mesh/Material/Camera/Light）
- **Transform 使用 `glm::quat`**，弧度制旋转

### 材质与着色器

- **材质图 (Material Graph)** — 可视化节点编辑器 → GLSL 片段编译器
- **着色器变体系统** — `ShaderTemplate`（`%snippet%` 占位符）+ `ShaderSnippet` → `ShaderLibrary::GetOrCreatePipelineVariant` 生成完整着色器
- **材质实例** — 运行时参数覆盖，自动 UBO/纹理绑定

### 编辑器 (AF-Editor)

- **基于 ImGui** — 多窗口停靠界面
- **编辑器相机** — WASD + 右键旋转自由相机
- **面板** — Viewport（实时渲染预览 + G-Buffer 附件查看）、Hierarchy（场景实体树）、Inspector
- **主题系统** — JSON 驱动的暗色主题，OpenSans 字体

### 其他

- **Assimp** 模型加载（glTF/OBJ/FBX 等）
- **Box2D** 2D 物理集成
- **调试输出**：spdlog

## 项目结构

```
AFEngine/
├── AFEngine/           # 引擎核心库 (STATIC)
│   └── src/
│       ├── Core/       # Application, Engine, System, Window, Input, Timer, UUID, Log
│       ├── Events/     # Event 基类 + 键盘/鼠标/窗口事件
│       ├── ECS/        # EnTT 封装，Entity/World/Components
│       ├── Platform/GLFW/  # GLFW 窗口 + 输入后端
│       ├── RHI/        # 抽象 GPU 接口 + OpenGL 实现 + CommandBuffer
│       ├── RenderGraph/# DAG 渲染图 (Node/Graph/Context/PSOCache)
│       ├── Renderer/   # RenderSystem, Camera, Mesh, RenderPipeline 基类
│       │   ├── Deferred/   # DeferredRenderPipeline (G-Buffer + Composite)
│       │   ├── Brdf/       # PBR BRDF (DefaultLit, ClearCoat)
│       │   └── ForwardRenderPipeline.*
│       ├── Material/   # Material, MaterialInstance, MaterialFactory, MaterialImporter
│       ├── MaterialGraph/ # 材质节点图 + →GLSL 编译器
│       ├── ShaderVariant/ # ShaderLibrary, ShaderTemplate, VariantKey
│       ├── Factory/    # MeshFactory (Triangle/Quad/Cube/Plane/Sphere/Cylinder/Capsule)
│       └── UI/         # ImGuiSystem + Theme 系统 (ThemeManager/ThemeSerializer)
├── AF-Editor/          # 编辑器应用程序 (EXE)
│   └── src/
│       ├── EditorApp.cpp
│       ├── EditorCamera
│       ├── EditorSystem
│       └── Panels/     # ViewportPanel + HierarchyPanel + InspectorPanel
└── Sandbox/            # 示例场景 (EXE)
    └── src/SandboxApp.cpp
```

## 构建

### 环境

- C++17 编译器 (Visual Studio 2022)
- CMake 3.16+
- Windows 10/11
- OpenGL 4.5+

### 依赖 (全部包含在 `vendor/`)

| 依赖 | 用途 |
|------|------|
| GLFW + GLAD | 窗口创建 + OpenGL 函数加载 |
| EnTT | ECS 框架 |
| ImGui + ImGuizmo | 编辑器 GUI + 操纵器 |
| GLM | 数学库 |
| Assimp (预编译 `.lib`) | 模型导入 |
| yaml-cpp | 配置文件解析 |
| Box2D | 2D 物理 |
| spdlog | 日志 |
| stb_image | 纹理加载 |
| nlohmann/json | JSON 序列化 |

### 命令

```powershell
# 配置
cmake -B build/vs -S . -G "Visual Studio 17 2022" -A x64

# 构建
cmake --build build/vs --config Debug

# 仅构建指定目标
cmake --build build/vs --target AFEngine --config Debug
cmake --build build/vs --target AF-Editor --config Debug
cmake --build build/vs --target Sandbox --config Debug

# 分发 (exe + assets + Resources)
cmake --install build/vs --config Debug --prefix build/install/Debug --component Runtime
```

构建产物：`AF-Editor.exe`、`Sandbox.exe`。

## 许可证

[MIT](LICENSE)
