#include "afpch.h"
#include "SceneRenderer.h"
#include "AF/Renderer/Renderer.h"
#include "AF/Renderer/Renderer2D.h"
#include "AF/Renderer/RenderPass.h"

#include <queue>

namespace AF {

	SceneRenderer::SceneRendererData SceneRenderer::s_Data;

	void SceneRenderer::Init()
	{
		InitRenderGraph();

		s_Data.DirLightBuffer.reserve(4);  // 预留4个方向光的空间
		s_Data.PointLightBuffer.reserve(16);  // 预留16个点光源的空间

		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(s_Data.CameraBuffer), 0);
		s_Data.DirLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(DirLight) * s_Data.DirLightBuffer.capacity(), 0);
		s_Data.PointLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(PointLight) * s_Data.PointLightBuffer.capacity(), 1);
	}

	void SceneRenderer::BeginScene(const Ref<Scene>& scene)
	{
		s_Data.ActiveScene = scene;

		glm::mat4 ViewMatrix = s_Data.ActiveScene->GetCamera()->GetViewMatrix();
		glm::mat4 ProjectionMatrix = s_Data.ActiveScene->GetCamera()->GetProjection();
		s_Data.CameraBuffer.ViewPosition = s_Data.ActiveScene->GetCamera()->GetPosition();
		s_Data.CameraBuffer.View = ViewMatrix;
		s_Data.CameraBuffer.ViewInverse = glm::inverse(ViewMatrix);
		s_Data.CameraBuffer.Projection = ProjectionMatrix;
		s_Data.CameraBuffer.ProjectionInverse = glm::inverse(ProjectionMatrix);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(s_Data.CameraBuffer));

		CollectSceneLights();
	}

	void SceneRenderer::EndScene()
	{
		FlushDrawList();

		s_Data.ActiveScene = nullptr;
	}

	void SceneRenderer::CollectSceneLights()
	{
		if (!s_Data.ActiveScene) return;

		s_Data.DirLightBuffer.clear();
		s_Data.PointLightBuffer.clear();

		// 收集平行光
		auto directionalView = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, DirectionalLightComponent>();
		for (auto entity : directionalView)
		{
			auto [transform, light] = directionalView.get<TransformComponent, DirectionalLightComponent>(entity);

			if (!light.Enabled) continue;

			// 计算方向：从变换位置指向世界中心 (0,0,0)
			glm::vec3 direction = glm::normalize(-transform.Translation);

			DirLight dirlight = {};
			dirlight.direction = direction;
			dirlight.ambient = light.Ambient;
			dirlight.diffuse = light.Diffuse;
			dirlight.specular = light.Specular;

			s_Data.DirLightBuffer.push_back(dirlight);
		}

		// 收集点光源
		auto pointView = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, PointLightComponent>();
		for (auto entity : pointView)
		{
			auto [transform, light] = pointView.get<TransformComponent, PointLightComponent>(entity);

			if (!light.Enabled) continue;

			PointLight pointlight = {};
			pointlight.position = transform.Translation;
			pointlight.color = light.Color;
			pointlight.intensity = light.Intensity;

			s_Data.PointLightBuffer.push_back(pointlight);
		}
	}

	void SceneRenderer::InitRenderGraph()
	{
		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Width = 1280;
		geoFramebufferSpec.Height = 720;
		geoFramebufferSpec.Attachments = {
			FramebufferTextureFormat::RGBA8,       // 0: Albedo (RGB) + Alpha (A)
			FramebufferTextureFormat::RED_INTEGER, // 1: EntityID
			FramebufferTextureFormat::RGBA8,     // 2: Normal (世界空间)
			FramebufferTextureFormat::RGBA8,       // 3: Metallic, Roughness, AO
			FramebufferTextureFormat::Depth        // 4: Depth
		};
		geoFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

		// Geometry Render Pass Specification
		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.TargetFramebuffer = Framebuffer::Create(geoFramebufferSpec);
		geoRenderPassSpec.m_Shader = Shader::Create("assets/shaders/geometry_pass.glsl");
		auto geoPass = RenderPass::Create(geoRenderPassSpec);

		AddRenderNode("Geometry", geoPass, [geoPass]() {
			// 几何通道
			// Clear our entity ID attachment to -1
			geoPass->GetSpecification().TargetFramebuffer->ClearAttachment(1, -1);

			// 渲染场景中的所有网格实体
			if (s_Data.ActiveScene) {
				// 对相同材质的网格进行分组以减少状态切换
				std::unordered_map<Ref<Material>, std::vector<std::tuple<Ref<Mesh>, glm::mat4, int>>> materialBatches;

				auto view = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, MeshComponent, MaterialComponent>();
				for (auto entity : view)
				{
					auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);

					materialBatches[material.material].push_back(std::make_tuple(mesh.mesh, transform.GetTransform(), (int)entity));
				}

				// 按材质批处理渲染
				for (const auto& [material, meshList] : materialBatches)
				{
					for (const auto& [mesh, transform, entityId] : meshList)
					{
						Renderer::SubmitMesh(mesh, material, transform, entityId);
					}
				}
			}

			Renderer2D::BeginScene(s_Data.ActiveScene->GetCamera());
			{
				// Draw sprites
				{
					auto group = s_Data.ActiveScene->GetAllEntitiesWithGroup<TransformComponent>(entt::get<SpriteRendererComponent>);
					for (auto entity : group)
					{
						auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

						Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
					}
				}

				// Draw circles
				{
					auto view = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, CircleRendererComponent>();
					for (auto entity : view)
					{
						auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

						Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
					}
				}

				// Draw text
				//{
				//	auto view = m_Registry.view<TransformComponent, TextComponent>();
				//	for (auto entity : view)
				//	{
				//		auto [transform, text] = view.get<TransformComponent, TextComponent>(entity);

				//		Renderer2D::DrawString(text.TextString, transform.GetTransform(), text, (int)entity);
				//	}
				//}
			}
			Renderer2D::EndScene();
			}, {}, { "GBufferAlbedo", "GBufferEntityID", "GBufferNormal", "GBufferMP", "GBufferDepth" });

		// Lighting Render Pass Specification
		FramebufferSpecification lightingFramebufferSpec;
		lightingFramebufferSpec.Width = 1280;
		lightingFramebufferSpec.Height = 720;
		lightingFramebufferSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F  // 0: Lighting Result
		};

		RenderPassSpecification lightingRenderPassSpec;
		lightingRenderPassSpec.TargetFramebuffer = Framebuffer::Create(lightingFramebufferSpec);
		lightingRenderPassSpec.m_Shader = Shader::Create("assets/shaders/lighting_pass.glsl");
		lightingRenderPassSpec.SetUniform("u_GAlbedo", 0);
		lightingRenderPassSpec.SetUniform("u_GNormal", 1);
		lightingRenderPassSpec.SetUniform("u_GMaterial", 2);
		lightingRenderPassSpec.SetUniform("u_GDepth", 3);
		auto lightingPass = RenderPass::Create(lightingRenderPassSpec);

		AddRenderNode("DeferredLighting", lightingPass, [lightingPass]() {
			Renderer::SubmitFullscreenQuad(nullptr);
			}, { "GBufferAlbedo", "GBufferNormal", "GBufferMP", "GBufferDepth" }, { "LightingResult" });

		FramebufferSpecification comFramebufferSpec;
		comFramebufferSpec.Width = 1280;
		comFramebufferSpec.Height = 720;
		comFramebufferSpec.Attachments = {
			FramebufferTextureFormat::RGBA8
		};
		geoFramebufferSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

		// Composite Render Pass Specification
		RenderPassSpecification comRenderPassSpec;
		comRenderPassSpec.TargetFramebuffer = Framebuffer::Create(comFramebufferSpec);
		comRenderPassSpec.m_Shader = Shader::Create("assets/shaders/hdr.glsl");
		comRenderPassSpec.SetUniform("u_Exposure", s_Data.Exposure);
		comRenderPassSpec.SetUniform("u_SceneTexture", 0);
		auto compPass = RenderPass::Create(comRenderPassSpec);

		AddRenderNode("Composite", compPass, [compPass]() {
			Renderer::SubmitFullscreenQuad(nullptr);
			}, { "LightingResult" }, { "FinalColor" });

		CompileRenderGraph();
	}

	void SceneRenderer::AddRenderNode(const std::string& name, Ref<RenderPass> pass,
		std::function<void()> executeFunc,
		const std::vector<std::string>& inputs,
		const std::vector<std::string>& outputs) {
		s_Data.RenderNodes.push_back({ name, pass, executeFunc, inputs, outputs });
	}

	void SceneRenderer::CompileRenderGraph()
	{
		// 清空之前的依赖关系
		s_Data.ResourceProducers.clear();
		s_Data.ResourceConsumers.clear();
		s_Data.ExecutionOrder.clear();

		// 建立资源生产者和消费者映射
		for (const auto& node : s_Data.RenderNodes) {
			for (const auto& output : node.outputs) {
				s_Data.ResourceProducers[output].push_back(node.name);
			}
			for (const auto& input : node.inputs) {
				s_Data.ResourceConsumers[input].push_back(node.name);
			}
		}

		// 验证依赖关系
		for (const auto& node : s_Data.RenderNodes) {
			for (const auto& input : node.inputs) {
				if (s_Data.ResourceProducers.find(input) == s_Data.ResourceProducers.end()) {
					AF_CORE_ERROR("RenderGraph: Input '{}' for node '{}' has no producer", input, node.name);
				}
			}
		}

		// 拓扑排序确定执行顺序
		std::unordered_map<std::string, int> inDegree;
		std::unordered_map<std::string, std::vector<std::string>> dependencies;

		// 初始化入度和依赖关系
		for (const auto& node : s_Data.RenderNodes) {
			inDegree[node.name] = 0;
		}

		for (const auto& node : s_Data.RenderNodes) {
			for (const auto& input : node.inputs) {
				for (const auto& producer : s_Data.ResourceProducers[input]) {
					if (producer != node.name) { // 避免自环
						dependencies[producer].push_back(node.name);
						inDegree[node.name]++;
					}
				}
			}
		}

		// 拓扑排序
		std::queue<std::string> q;
		for (const auto& [name, degree] : inDegree) {
			if (degree == 0) {
				q.push(name);
			}
		}

		while (!q.empty()) {
			std::string current = q.front();
			q.pop();
			s_Data.ExecutionOrder.push_back(current);

			for (const auto& dependent : dependencies[current]) {
				if (--inDegree[dependent] == 0) {
					q.push(dependent);
				}
			}
		}

		// 检查是否有环
		if (s_Data.ExecutionOrder.size() != s_Data.RenderNodes.size()) {
			AF_CORE_ERROR("RenderGraph: Cycle detected in render graph dependencies!");
			s_Data.ExecutionOrder.clear();
			// 回退到声明顺序
			for (const auto& node : s_Data.RenderNodes) {
				s_Data.ExecutionOrder.push_back(node.name);
			}
		}

		s_Data.GraphCompiled = true;
		UpdateGraphResources();
	}

	void SceneRenderer::UpdateGraphResources()
	{
		// 更新所有节点的输出资源
		for (const auto& node : s_Data.RenderNodes) {
			UpdateNodeOutputs(node);
		}
	}

	void SceneRenderer::UpdateNodeOutputs(const RenderGraphNode& node)
	{
		auto framebuffer = node.pass->GetSpecification().TargetFramebuffer;

		// 更新颜色附件映射
		for (size_t i = 0; i < node.outputs.size() && i < framebuffer->GetColorAttachmentCount(); ++i) {
			s_Data.GraphResources[node.outputs[i]] = framebuffer->GetColorAttachment(i);
		}

		// 更新深度附件映射
		if (framebuffer->GetDepthAttachment()) {
			for (size_t i = framebuffer->GetColorAttachmentCount(); i < node.outputs.size(); ++i) {
				if (node.outputs[i].find("Depth") != std::string::npos ||
					node.outputs[i].find("depth") != std::string::npos) {
					s_Data.GraphResources[node.outputs[i]] = framebuffer->GetDepthAttachment();
					break;
				}
			}
		}
	}

	void SceneRenderer::ExecuteRenderGraph()
	{
		if (!s_Data.GraphCompiled) {
			AF_CORE_ERROR("Render Graph not compiled");
			return;
		}

		UpdateGraphResources();

		// 按拓扑排序顺序执行
		for (const auto& nodeName : s_Data.ExecutionOrder) {
			// 查找节点
			auto it = std::find_if(s_Data.RenderNodes.begin(), s_Data.RenderNodes.end(),
				[&](const RenderGraphNode& node) { return node.name == nodeName; });

			if (it == s_Data.RenderNodes.end()) continue;

			auto& node = *it;

			Renderer::BeginRenderPass(node.pass);

			if (node.clearOnExecute) {
				RenderCommand::SetClearColor(node.pass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor);
				RenderCommand::Clear();
			}

			// 自动绑定输入纹理
			BindNodeInputs(node.inputs);

			//全局参数
			s_Data.CameraUniformBuffer->Bind();

			s_Data.DirLightUniformBuffer->SetData(
				s_Data.DirLightBuffer.empty() ? nullptr : s_Data.DirLightBuffer.data(),
				sizeof(DirLight) * s_Data.DirLightBuffer.size()
			);
			s_Data.DirLightUniformBuffer->Bind();

			s_Data.PointLightUniformBuffer->SetData(
				s_Data.PointLightBuffer.empty() ? nullptr : s_Data.PointLightBuffer.data(),
				sizeof(PointLight) * s_Data.PointLightBuffer.size()
			);
			s_Data.PointLightUniformBuffer->Bind();

			// 执行渲染逻辑
			node.executeFunction();

			Renderer::EndRenderPass();

			// 更新输出资源映射（处理可能的附件变化）
			UpdateNodeOutputs(node);
		}
	}

	void SceneRenderer::BindNodeInputs(const std::vector<std::string>& inputs)
	{
		static Ref<Texture2D> defaultTexture = Texture2D::Create("assets/textures/defaultTexture.jpg");

		for (size_t textureUnit = 0; textureUnit < inputs.size(); ++textureUnit) {
			const auto& inputName = inputs[textureUnit];
			auto it = s_Data.GraphResources.find(inputName);
			if (it != s_Data.GraphResources.end() && it->second) {
				it->second->Bind(textureUnit);
				//AF_CORE_TRACE("Bound input '{}' to texture unit {}", inputName, textureUnit);
			}
			else {
				AF_CORE_WARN("RenderGraph: Input resource '{}' not found", inputName);
				// 绑定一个默认纹理避免着色器错误
				if (defaultTexture) {
					defaultTexture->Bind(textureUnit);
				}
			}
		}
	}

	void SceneRenderer::FlushDrawList()
	{
		ExecuteRenderGraph();
	}

	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		for (auto& node : s_Data.RenderNodes) {
			node.pass->GetSpecification().TargetFramebuffer->Resize(width, height);
		}

		// 视口改变后需要更新所有资源引用
		UpdateGraphResources();
	}

	Ref<Texture2D> SceneRenderer::GetFinalColorBuffer()
	{
		auto it = s_Data.GraphResources.find("FinalColor");
		if (it != s_Data.GraphResources.end() && it->second) {
			return it->second;
		}

		AF_CORE_ERROR("FinalColor buffer not found in render graph");
		return nullptr;
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		auto it = s_Data.GraphResources.find("FinalColor");
		if (it != s_Data.GraphResources.end() && it->second) {
			return it->second->GetRendererID();
		}

		AF_CORE_ERROR("FinalColor buffer not found in render graph");
		return 0;
	}

	int SceneRenderer::ReadPixel(int x, int y)
	{
		// 通过名称查找Geometry Pass
		for (auto& node : s_Data.RenderNodes) {
			if (node.name == "Geometry") {
				auto framebuffer = node.pass->GetSpecification().TargetFramebuffer;
				framebuffer->Bind();
				// 从几何通道的第二个颜色附件（实体ID）读取像素
				int pixelData = framebuffer->ReadPixel(1, x, y);
				framebuffer->Unbind();
				return pixelData;
			}
		}
		return -1;
	}

}