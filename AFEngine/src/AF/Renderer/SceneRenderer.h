#pragma once
#include "AF/Scene/Scene.h"

#include "AF/Renderer/RenderPass.h"
#include "AF/Renderer/UniformBuffer.h"
#include <string>
#include <vector>
#include <functional>

namespace AF {

	/**
	 * @class SceneRenderer
	 * @brief 高层级场景渲染器，负责驱动完整的延迟渲染管线 (Deferred Rendering Pipeline)。
	 * 
	 * 该类通过“渲染图 (Render Graph)”架构组织渲染流程，核心职责：
	 * 1. **管线编排**：定义 G-Buffer、阴影、光照、后处理等阶段的顺序与依赖。
	 * 2. **资源调度**：管理纹理池 (GraphResources)，在节点间自动传递 Framebuffer 附件。
	 * 3. **GPU 数据同步**：实时更新并同步相机 (UBO) 和光源 (SSBO) 的数据到 GPU。
	 * 4. **空间计算**：计算阴影矩阵、处理视口缩放以及像素拾取。
	 */
	class SceneRenderer
	{
	public:
		/**
		 * @struct RenderGraphNode
		 * @brief 渲染图中的逻辑执行单元。
		 * 每个节点通常代表一个完整的渲染阶段 (Render Pass)。
		 */
		struct RenderGraphNode {
			std::string name;                     ///< 节点唯一名称
			Ref<RenderPass> pass;                 ///< 关联的渲染资源 (Shader + Framebuffer)
			std::function<void()> executeFunction;///< 该阶段的具体渲染逻辑回调 (SubmitMesh 等调用发生在此)
			std::vector<std::string> inputs;      ///< 依赖的输入资源名 (从全局资源池采样)
			std::vector<std::string> outputs;     ///< 产出的输出资源名 (注册到全局资源池)
			
			int executeInterval = 1;              ///< 执行频率 (1: 每帧, 0: 仅一次, N: 每 N 帧)
			bool clearOnExecute = true;           ///< 是否在执行前自动清除 FB
			mutable int frameCounter = 0;         ///< 频率控制计数器

			/** @brief 频率控制逻辑 */
			bool shouldExecute() const {
				if (executeInterval == 0) {
					if (frameCounter) return true;
					frameCounter = 1;
				}
				else if (executeInterval > 1) {
					frameCounter++;
					if (frameCounter < executeInterval) return true;
					frameCounter = 0;
				}
				return false;
			}
		};

	public:
		// ===================================================================================
		// --- 核心生命周期接口 (Core Lifecycle) ---
		// ===================================================================================
		
		/**
		 * @brief 初始化场景渲染系统。
		 * 分配持久化的 UBO/SSBO 缓冲区、创建阴影贴图数组并构建初始渲染图架构。
		 */
		static void Init(); 

		/**
		 * @brief 响应视口缩放。
		 * 自动重建所有受影响节点的 Framebuffer 附件，并刷新资源引用。
		 */
		static void OnViewportResize(uint32_t width, uint32_t height);

		// ===================================================================================
		// --- 渲染驱动接口 (Execution Flow) ---
		// ===================================================================================
		
		/**
		 * @brief 开始场景渲染。
		 * 锁定当前场景，上传最新的相机矩阵，并遍历场景收集灯光信息。
		 */
		static void BeginScene(const Ref<Scene>& scene);

		/**
		 * @brief 提交并执行。
		 * 触发渲染图的自动编译（若有变更）并按拓扑顺序派发 GPU 指令。
		 */
		static void EndScene();

		// ===================================================================================
		// --- 渲染图管理 (Graph Management) ---
		// ===================================================================================
		
		/**
		 * @brief 动态添加渲染阶段。
		 */
		static void AddRenderNode(const std::string& name, Ref<RenderPass> pass,
			std::function<void()> executeFunc,
			const std::vector<std::string>& inputs = {},
			const std::vector<std::string>& outputs = {},
			const int executeInterval = 1);

		// ===================================================================================
		// --- 数据拾取与结果输出 (Data Access) ---
		// ===================================================================================
		
		/**
		 * @brief 获取渲染图中指定名称的资源纹理。
		 * @param name 资源名称 (如 "FinalColor", "GBufferAlbedo", "DirShadowMap")
		 */
		static Ref<Texture> GetBuffer(const std::string& name = "FinalColor");

		/**
		 * @brief 获取指定资源的 OpenGL Renderer ID。
		 * @param name 资源名称
		 */
		static uint32_t GetBufferRendererID(const std::string& name = "FinalColor");

		/**
		 * @brief 在屏幕坐标处读取像素 ID。
		 * 用于基于 ID 的物体拾取 (Entity Picking)。
		 */
		static int ReadPixel(int x, int y);

		static const std::vector<RenderGraphNode>& GetRenderNodes() { return s_Data.RenderNodes; }

      static bool &GetEnableSSGIRef() { return s_Data.EnableSSGI; }

	private:
		// ===================================================================================
		// --- 渲染图驱动逻辑 (Internal Graph Engine) ---
		// ===================================================================================
		
		/** @brief 预置默认渲染管线 (G-Buffer -> Shadow -> Lighting -> PostProcess) */
		static void InitRenderGraph();

		/** @brief 编译：通过 Kahn 算法分析依赖并生成拓扑排序序列 */
		static void CompileRenderGraph();

		/** @brief 执行：循环派发 Pass 指令，处理 Uniform 自动绑定 */
		static void ExecuteRenderGraph();

		/** @brief 资源刷新：确保 Shader 采样器引用的纹理 ID 与最新的 Framebuffer 附件同步 */
		static void UpdateGraphResources();

		/** @brief 自动绑定：根据 node.inputs 将资源池中的纹理绑定到 Shader Uniform */
		static void BindNodeInputs(const RenderGraphNode& node);

		/** @brief 结果导出：将 Framebuffer 的附件按 node.outputs 名存入全局资源池 */
		static void UpdateNodeOutputs(const RenderGraphNode& node);

		// ===================================================================================
		// --- 场景预处理 (Preprocessing) ---
		// ===================================================================================
		
		/** @brief 收集光源：遍历 ECS 实体，生成光源数据镜像并计算阴影投影矩阵 */
		static void CollectSceneLights();

	private:
		// ===================================================================================
		// --- GPU 对应数据结构 (GPU Buffer Mirrors) ---
		// ===================================================================================

		/** @brief 相机数据块 (对应 GLSL std140 layout) */
		struct CameraData
		{
			glm::vec3 ViewPosition;       ///< 世界空间相机位置
			unsigned int padding;
			glm::mat4 View;               ///< 视图矩阵
			glm::mat4 ViewInverse;        ///< 逆视图矩阵
			glm::mat4 Projection;         ///< 投影矩阵
			glm::mat4 ProjectionInverse;  ///< 逆投影矩阵
		};

		/** @brief 方向光数据块 (对应 GLSL std430 layout) */
		struct DirLight
		{
			glm::vec3 direction;          ///< 光照方向 (Normalized)
			float pad1;
			glm::vec3 ambient;            ///< 环境光强度
			float pad2;
			glm::vec3 diffuse;            ///< 漫反射颜色
			float pad3;
			glm::vec3 specular;           ///< 高光颜色
			float pad4;
			glm::mat4 LightSpaceMatrix;   ///< 阴影变换矩阵 (VP)
		};

		/** @brief 点光源数据块 (对应 GLSL std430 layout) */
		struct PointLight
		{
			glm::vec3 position;           ///< 世界空间位置
			float padding1;
			glm::vec3 color;              ///< 光照颜色
			float intensity;              ///< 能量强度
			glm::mat4 LightSpaceMatrix[6];///< 立方体阴影贴图的 6 个投影矩阵
		};

		/** @brief 场景渲染器私有状态容器 */
		struct SceneRendererData
		{
			Ref<Scene> ActiveScene;              ///< 当前渲染的目标场景
			Ref<Texture2D> EnvMap;               ///< IBL/天空盒纹理
			float Exposure = 1.0f;               ///< HDR 曝光值
			bool EnableSSGI = true;             ///< 是否启用 SSGI

			// --- 渲染图状态 ---
			std::vector<RenderGraphNode> RenderNodes;
			std::unordered_map<std::string, Ref<Texture>> GraphResources;
			bool GraphCompiled = false;
			std::unordered_map<std::string, std::vector<std::string>> ResourceProducers;
			std::unordered_map<std::string, std::vector<std::string>> ResourceConsumers;
			std::vector<std::string> ExecutionOrder;

			// --- 持久化 GPU 资源 ---
			Ref<Texture2D> DirShadowMapArray;    ///< 方向光深度图数组 (CSM 基础)
			Ref<TextureCube> PointShadowMapArray;///< 点光源立方体深度图数组

			// --- GPU 缓冲区镜像 ---
			CameraData CameraBuffer;
			Ref<UniformBuffer> CameraUniformBuffer;

			std::vector<DirLight> DirLightBuffer;
			Ref<ShaderStorageBuffer> DirLightUniformBuffer;
			
			std::vector<PointLight> PointLightBuffer;
			Ref<ShaderStorageBuffer> PointLightUniformBuffer;
		};

		static SceneRendererData s_Data;
	};

}