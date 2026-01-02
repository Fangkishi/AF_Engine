# AFEngine

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/YourUsername/AFEngine)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

一个基于 C++ 的现代化高性能游戏引擎练习项目。

## 简介

AFEngine 是一个使用 C++17 编写的现代化游戏引擎。项目经过重构，目前已达到 **v1.0** 里程碑。该版本引入了先进的渲染架构和优化的系统设计，旨在提供一个功能完整、架构清晰的 3D/2D 游戏开发环境。

## 主要特性 (v1.0)

### 1. 渲染系统 (Renderer)
*   **渲染图 (Render Graph) 架构**: 
    *   解耦渲染阶段，通过有向无环图 (DAG) 管理 Pass 依赖。
    *   自动资源调度与生命周期管理（纹理池、缓冲区同步）。
    *   支持动态添加自定义渲染节点。
*   **延迟渲染管线 (Deferred Rendering)**:
    *   **G-Buffer 阶段**: 提取颜色、法线、材质参数及实体 ID。
    *   **阴影阶段**: 支持平行光级联阴影与点光源全景阴影。
    *   **光照阶段**: 基于物理的渲染 (PBR)，支持 HDR、曝光控制。
*   **性能优化**:
    *   **状态追踪 (State Tracking)**: 极大减少冗余的 VAO 切换与 Uniform 更新。
    *   **绘制排序**: 自动按材质和网格排序，最小化状态切换开销。
    *   **纹理缓存**: 智能纹理槽位分配与去重。
*   **2D 渲染器**: 支持高性能批处理渲染（精灵、几何体）。

### 2. 场景系统 (Scene)
*   **基于 ECS 架构**: 使用 EnTT 库，实现高效的组件式开发。
*   **核心组件库**: Transform, Mesh, Material, Camera, Light, Sprite 等。
*   **序列化系统**: 基于 YAML 的场景与项目保存/加载功能。

### 3. 编辑器 (AF-Editor)
*   **可视化工作流**: 基于 ImGui 的多窗口停靠界面。
*   **实时预览**: 支持在视口中查看任意渲染图节点的中间结果（如 G-Buffer 各附件、阴影图）。
*   **物体拾取**: 基于 GPU 缓冲区的精确像素拾取系统。
*   **资源管理**: 集成内容浏览器、场景层次结构面板与属性检查器。

### 4. 资源加载 (Loader)
*   **模型加载**: 通过 Assimp 支持主流 3D 模型格式。
*   **着色器系统**: 支持自定义宏预处理与热加载。

## 项目结构

```bash
AFEngine/
├── AFEngine/           # 引擎核心库
│   └── src/AF/
│       ├── Core/       # 应用框架与底层抽象
│       ├── Renderer/   # 核心渲染架构（渲染图、PBR、延迟渲染）
│       ├── Scene/      # ECS 场景管理
│       ├── Loader/     # 模型与纹理加载
│       └── Physics/    # 物理系统接口
├── AF-Editor/          # 现代化编辑器应用
└── Sandbox/            # 示例项目
```

## 构建说明

### 环境要求
*   支持 C++17 的编译器 (Visual Studio 2019+, GCC 9+, Clang 10+)
*   CMake 3.16+
*   Windows 10/11

### 依赖项
*   **OpenGL 4.5+** (渲染后端)
*   **GLFW / GLAD** (窗口与函数加载)
*   **EnTT** (高性能 ECS)
*   **ImGui / ImGuizmo** (编辑器界面与 Gizmos)
*   **GLM** (数学库)
*   **Assimp** (模型解析)
*   **yaml-cpp** (配置文件处理)
*   **Box2D** (2D 物理系统)

### 快速开始
1. 克隆仓库。
2. 使用 CMake 生成构建文件：
   ```bash
   mkdir build
   cd build
   cmake ..
   ```
3. 编译项目：
   - **Windows (Visual Studio)**: 打开 `build/AFEngine.sln`，将 `AF-Editor` 设为启动项目并运行。
   - **命令行**: `cmake --build . --config Release`
4. 运行 `AF-Editor` 即可开始使用。

## 开发规划
- [ ] 多线程渲染以及命令队列
- [ ] 集成 3D 物理引擎 (PhysX 或 Jolt)
- [ ] 增加骨骼动画支持
- [ ] 引入 C# 脚本系统
- [ ] 完善后处理栈 (Bloom, SSAO, SSR)

## 许可证
本项目采用 [MIT 许可证](LICENSE) 发布。
