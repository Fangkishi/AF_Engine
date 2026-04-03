#pragma once

#include "RenderCommand.h"
#include "AF/Renderer/SceneRenderer.h"
#include "AF/Renderer/RenderPass.h"

#include "AF/Renderer/Camera.h"
#include "AF/Renderer/EditorCamera.h"

#include "AF/Renderer/Mesh.h"
#include "AF/Renderer/Material.h"
#include "AF/Renderer/UniformBuffer.h"
#include "Shader.h"

namespace AF {

	/**
	 * @class Renderer
	 * @brief 核心渲染器类，提供底层的渲染指令提交、材质应用以及渲染状态管理。
	 * 
	 * 该类作为引擎渲染系统的底层入口（Low-level API Wrapper），主要职责包括：
	 * 1. **状态追踪 (State Tracking)**：维护当前绑定的网格、材质和纹理状态，通过跳过冗余调用优化性能。
	 * 2. **材质应用 (Material Application)**：统一处理 Shader 参数绑定，包括全局、网格、材质及实例级 Uniform。
	 * 3. **绘制提交 (Draw Submission)**：封装各种几何体（网格、全屏四边形等）的底层 Draw Call 派发。
	 * 4. **生命周期管理**：协调 Renderer2D, SceneRenderer 等子系统的初始化与关闭。
	 */
	class Renderer
	{
	public:
		// ===================================================================================
		// --- 基础生命周期管理 (System Lifecycle) ---
		// ===================================================================================
		
		/**
		 * @brief 初始化全局渲染资源。
		 * 依次初始化 RenderCommand (API配置), Renderer2D (批处理渲染器), SceneRenderer (高层管线)，
		 * 并预加载引擎内置的默认材质、默认着色器以及全屏绘制所需的几何数据。
		 */
		static void Init();

		/**
		 * @brief 释放所有全局渲染资源。
		 */
		static void Shutdown();

		/**
		 * @brief 响应视口/窗口尺寸变化。
		 * 同步更新底层渲染 API 的视口设置。
		 * @param width 新的视口宽度
		 * @param height 新的视口高度
		 */
		static void OnWindowResize(uint32_t width, uint32_t height);

		// ===================================================================================
		// --- 渲染通道 (Render Pass) 流程控制 ---
		// ===================================================================================

		/**
		 * @brief 启动一个渲染通道。
		 * 绑定目标 Framebuffer，激活关联 Shader，同步 Pass 级全局 Uniform，并重置状态追踪缓存（如 ActiveMesh/Material）。
		 * @param renderPass 包含目标缓冲、着色器配置及全局参数的通道对象
		 */
		static void BeginRenderPass(const Ref<RenderPass> renderPass);

		/**
		 * @brief 结束当前正在执行的渲染通道。
		 * 解绑当前 Framebuffer 并重置纹理槽位计数器。
		 */
		static void EndRenderPass();

		// ===================================================================================
		// --- 几何体提交 (Drawing Submission) ---
		// ===================================================================================

		/**
		 * @brief 提交 3D 几何网格绘制。
		 * 核心绘制逻辑：
		 * 1. 自动判断并仅在变更时绑定材质/网格 Uniform。
		 * 2. 应用实例私有数据（如 Transform）。
		 * 3. 触发底层索引绘制。
		 * @param mesh 目标网格资源（包含 VAO 和 顶点数据）
		 * @param overridematerial 覆盖材质（若为 null 则使用全局默认材质）
		 * @param instanceUniforms 包含 Transform、EntityID 等实例级私有 Uniform 的容器
		 */
		static void SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const UniformContainer& instanceUniforms);

		/**
		 * @brief 提交阴影图渲染专用的深度绘制。
		 * 相比 SubmitMesh，该方法禁用了材质颜色的计算，仅保留深度信息和变换矩阵，以提升阴影渲染性能。
		 * @param mesh 目标网格资源
		 * @param instanceUniforms 包含变换数据的容器
		 */
		static void SubmitShadow(const Ref<Mesh>& mesh, const UniformContainer& instanceUniforms);

		/**
		 * @brief 提交全屏四边形绘制。
		 * 常用于延迟渲染的光照阶段、后处理滤镜或 UI 混合。
		 */
		static void SubmitFullscreenQuad();

		// ===================================================================================
		// --- 系统查询与底层访问 (Utilities) ---
		// ===================================================================================

		/**
		 * @brief 获取当前正在使用的底层渲染 API 类型 (如 OpenGL, Vulkan 等)。
		 */
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		/**
		 * @brief 底层渲染指令派发中枢。
		 * 所有渲染命令通过此函数提交，未来可扩展为命令队列 (Command Queue) 以支持异步渲染。
		 */
		static void Submit(const std::function<void()>& renderFunc);
	};
}
