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
			bool clearOnExecute = true;
		};

		static void Init(); 

		static void OnViewportResize(uint32_t width, uint32_t height);

		static void BeginScene(const Ref<Scene>& scene);
		static void EndScene();

		static void CollectSceneLights();

		static Ref<Texture2D> GetFinalColorBuffer();

		static uint32_t GetFinalColorBufferRendererID();

		static int ReadPixel(int x, int y);
	private:

		static void InitRenderGraph();
		static void CompileRenderGraph();
		static void AddRenderNode(const std::string& name, Ref<RenderPass> pass,
			std::function<void()> executeFunc,
			const std::vector<std::string>& inputs = {},
			const std::vector<std::string>& outputs = {});

		static void ExecuteRenderGraph();
		static void UpdateGraphResources(); // 每帧更新资源映射
		static void UpdateNodeOutputs(const RenderGraphNode& node);
		static void BindNodeInputs(const std::vector<std::string>& inputs);

		static void FlushDrawList();
	private:

		struct DirLight
		{
			glm::vec3 direction = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
			float pad1;
			glm::vec3 ambient = glm::vec3(0.2f);
			float pad2;
			glm::vec3 diffuse = glm::vec3(1.5f);
			float pad3;
			glm::vec3 specular = glm::vec3(1.0f);
			float pad4;
		};

		struct PointLight
		{
			glm::vec3 position = glm::vec3(3.0f, 0.0f, 0.0f);
			float padding1 = 0.0f;
			glm::vec3 color = glm::vec3(10.0f, 10.0f, 10.0f);
			float intensity = 5.0f;
		};

		struct SceneRendererData
		{
			Ref<Scene> ActiveScene;

			std::vector<RenderGraphNode> RenderNodes;
			std::unordered_map<std::string, Ref<Texture2D>> GraphResources;
			bool GraphCompiled = false;

			float Exposure = 1.0f;
			struct CameraData
			{
				glm::vec3 ViewPosition;       // 相机世界空间位置 (16字节：vec3 + padding)
				unsigned int padding;             // 对齐填充（使总大小为16字节）
				glm::mat4 View;               // 视图矩阵（世界→视图）(64字节)
				glm::mat4 ViewInverse;        // 视图逆矩阵（视图→世界）(64字节)
				glm::mat4 Projection;         // 投影矩阵（视图→裁剪）(64字节)
				glm::mat4 ProjectionInverse;  // 投影逆矩阵（裁剪→视图）(64字节)
			};
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