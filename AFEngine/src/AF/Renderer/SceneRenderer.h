#pragma once
#include "AF/Scene/Scene.h"

#include "AF/Renderer/RenderPass.h"
#include "AF/Renderer/UniformBuffer.h"

namespace AF {

	class SceneRenderer
	{
	public:

		struct RenderGraphNode {
			std::string name;
			Ref<RenderPass> pass;
			std::function<void()> executeFunction;
			std::vector<std::string> inputs;
			std::vector<std::string> outputs;
			int executeInterval = 1;        // 0：只执行一次、1：每帧执行、N：N帧执行
			bool clearOnExecute = true;
			mutable int frameCounter = 0;

			bool shouldExecute() const {
				if (executeInterval == 0)
				{
					if (frameCounter) {
						return true;
					}
					frameCounter = 1;
				}
				else if (executeInterval > 1)
				{
					frameCounter++;
					if (frameCounter < executeInterval) {
						return true;
					}
					frameCounter = 0; // 重置计数器
				}
				return false;
			}
		};

		static void Init(); 

		static void OnViewportResize(uint32_t width, uint32_t height);

		static void BeginScene(const Ref<Scene>& scene);
		static void EndScene();

		static void AddRenderNode(const std::string& name, Ref<RenderPass> pass,
			std::function<void()> executeFunc,
			const std::vector<std::string>& inputs = {},
			const std::vector<std::string>& outputs = {},
			const int executeInterval = 1);

		static Ref<Texture2D> GetFinalColorBuffer();

		static uint32_t GetFinalColorBufferRendererID();

		static int ReadPixel(int x, int y);
	private:
		static void CollectSceneLights();

		static void InitRenderGraph();
		static void CompileRenderGraph();


		static void ExecuteRenderGraph();
		static void UpdateGraphResources(); // 每帧更新资源映射
		static void BindNodeInputs(const RenderGraphNode& node);
		static void UpdateNodeOutputs(const RenderGraphNode& node);

	private:

		struct CameraData
		{
			glm::vec3 ViewPosition;       // 相机世界空间位置 (16字节：vec3 + padding)
			unsigned int padding;             // 对齐填充（使总大小为16字节）
			glm::mat4 View;               // 视图矩阵（世界→视图）(64字节)
			glm::mat4 ViewInverse;        // 视图逆矩阵（视图→世界）(64字节)
			glm::mat4 Projection;         // 投影矩阵（视图→裁剪）(64字节)
			glm::mat4 ProjectionInverse;  // 投影逆矩阵（裁剪→视图）(64字节)
		};

		struct DirLight
		{
			glm::vec3 direction = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
			float pad1;
			glm::vec3 ambient = glm::vec3(0.2f);
			float pad2;
			glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
			float pad3;
			glm::vec3 specular = glm::vec3(0.8f, 0.8f, 0.8f);
			float pad4;
			glm::mat4 LightSpaceMatrix;
		};

		struct PointLight
		{
			glm::vec3 position = glm::vec3(3.0f, 0.0f, 0.0f);
			float padding1 = 0.0f;
			glm::vec3 color = glm::vec3(10.0f, 10.0f, 10.0f);
			float intensity = 5.0f;
			glm::mat4 LightSpaceMatrix[6];
		};

		struct SceneRendererData
		{
			Ref<Scene> ActiveScene;

			Ref<Texture2D> EnvMap;

			std::vector<RenderGraphNode> RenderNodes;
			std::unordered_map<std::string, Ref<Texture>> GraphResources;
			bool GraphCompiled = false;

			float Exposure = 1.0f;

			// Shadowmap相关
			Ref<Texture2D> DirShadowMapArray;  // 用于方向光CSM
			Ref<TextureCube> PointShadowMapArray; // 用于点光阴影

			CameraData CameraBuffer;
			Ref<UniformBuffer> CameraUniformBuffer;

			std::vector<DirLight> DirLightBuffer;
			Ref<ShaderStorageBuffer> DirLightUniformBuffer;

			std::vector<PointLight> PointLightBuffer;
			Ref<ShaderStorageBuffer> PointLightUniformBuffer;

			// 依赖管理
			std::unordered_map<std::string, std::vector<std::string>> ResourceProducers; // 资源 -> 生产节点
			std::unordered_map<std::string, std::vector<std::string>> ResourceConsumers; // 资源 -> 消费节点
			std::vector<std::string> ExecutionOrder; // 拓扑排序后的执行顺序
		};

		static SceneRendererData s_Data;
	};

}