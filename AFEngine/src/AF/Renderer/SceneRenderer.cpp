#include "afpch.h"
#include "SceneRenderer.h"
#include "AF/Renderer/Renderer.h"
#include "AF/Renderer/Renderer2D.h"
#include "AF/Renderer/RenderPass.h"

#include <queue>
#include <glad/glad.h>

namespace AF {

	SceneRenderer::SceneRendererData SceneRenderer::s_Data;

	void SceneRenderer::Init()
	{
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(s_Data.CameraBuffer), 0);

		s_Data.DirLightBuffer.reserve(4);  // 预留4个方向光的空间
		s_Data.PointLightBuffer.reserve(16);  // 预留16个点光源的空间

		s_Data.DirLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(DirLight) * s_Data.DirLightBuffer.capacity(), 0);
		s_Data.PointLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(PointLight) * s_Data.PointLightBuffer.capacity(), 1);

		// 初始化 Shadowmap 纹理数组
		TextureSpecification dirShadowMapSpec;
		dirShadowMapSpec.Width = 2048;
		dirShadowMapSpec.Height = 2048;
		dirShadowMapSpec.Format = ImageFormat::Depth;
		dirShadowMapSpec.ArraySize = 4;

		s_Data.DirShadowMapArray = Texture2D::Create(dirShadowMapSpec);

		TextureSpecification pointShadowMapSpec;
		pointShadowMapSpec.Width = 2048;
		pointShadowMapSpec.Height = 2048;
		pointShadowMapSpec.Format = ImageFormat::Depth;
		pointShadowMapSpec.ArraySize = 16;

		s_Data.PointShadowMapArray = TextureCube::Create(pointShadowMapSpec);

		s_Data.EnvMap = Texture2D::Create("assets/textures/bk.jpg");

		InitRenderGraph();
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
		CompileRenderGraph();

		ExecuteRenderGraph();

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

			// 1. 方向计算：从变换的旋转中获取前向向量 (0, 0, -1)
			// 这样旋转光源实体就可以改变光照方向，而不是仅仅依靠位置
			glm::vec3 direction = glm::normalize(glm::rotate(glm::quat(transform.Rotation), glm::vec3(0.0f, 0.0f, -1.0f)));

			DirLight dirlight = {};
			dirlight.direction = direction;
			dirlight.ambient = light.Ambient;
			dirlight.diffuse = light.Diffuse;
			dirlight.specular = light.Specular;

			// 2. 构造光源视图矩阵
			// 平行光没有位置概念，但阴影图需要一个投影中心。通常让它跟随主相机。
			glm::vec3 cameraPos = s_Data.ActiveScene->GetCamera()->GetPosition();
			
			// 将光源“位置”放在相机位置，并沿光照反方向偏移，以包含相机前后的物体
			float shadowDistance = 100.0f;
			glm::vec3 lightPos = cameraPos - direction * (shadowDistance * 0.5f);
			glm::vec3 lightTarget = cameraPos;
			glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);

			// 如果光源方向与上方向过于接近，需要调整上方向以避免万向锁
			if (glm::abs(glm::dot(direction, lightUp)) > 0.99f) {
				lightUp = glm::vec3(0.0f, 0.0f, 1.0f);
			}

			glm::mat4 lightViewMatrix = glm::lookAt(lightPos, lightTarget, lightUp);

			// 3. 构造正交投影矩阵
			// orthoSize 决定了阴影覆盖的宽度和高度，1000.0f 的远近平面通常足以覆盖场景
			float orthoSize = 40.0f; 
			glm::mat4 cascadeProj = glm::ortho(
				-orthoSize, orthoSize,    // 左，右
				-orthoSize, orthoSize,    // 下，上
				0.1f, shadowDistance      // 近，远平面
			);

			// 光照空间矩阵
			dirlight.LightSpaceMatrix = cascadeProj * lightViewMatrix;

			//// 为每个级联计算光照空间矩阵（投影 * 视图）
			//for (uint32_t cascade = 0; cascade < 4; cascade++)
			//{
			//	// 计算级联的近远平面
			//	float cascadeNear = (cascade == 0) ? nearPlane : splitDepths[cascade - 1];
			//	float cascadeFar = splitDepths[cascade];

			//	// 调整级联投影矩阵的近/远平面
			//	float orthoSize = 50.0f;
			//	glm::mat4 cascadeProj = glm::ortho(
			//		-orthoSize, orthoSize,    // 左，右
			//		-orthoSize, orthoSize,    // 下，上
			//		cascadeNear, cascadeFar   // 近，远平面
			//	);

			//	// 光照空间矩阵 = 级联投影 * 光源视图
			//	dirshadow.LightSpaceMatrix[cascade] = cascadeProj * lightViewMatrix;
			//}

			s_Data.DirLightBuffer.push_back(dirlight);
		}

		// 收集点光源
		auto pointView = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, PointLightComponent>();
		for (auto entity : pointView)
		{
			auto [transform, light] = pointView.get<TransformComponent, PointLightComponent>(entity);

			if (!light.Enabled) continue;

			PointLight pointlight = {};
			glm::vec3 lightPos = transform.Translation;
			pointlight.position = lightPos;
			pointlight.color = light.Color;
			pointlight.intensity = light.Intensity;



			glm::mat4 shadowProj = glm::perspective(
				glm::radians(90.0f),  // 90度FOV，正好覆盖立方体一个面
				1.0f,          // 1:1 宽高比
				0.1f,            // 近平面
				100.0f              // 远平面
			);

			// 计算6个立方体面的光照空间矩阵（投影 * 视图）
			pointlight.LightSpaceMatrix[0] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));   // +X
			pointlight.LightSpaceMatrix[1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));  // -X
			pointlight.LightSpaceMatrix[2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));    // +Y
			pointlight.LightSpaceMatrix[3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));  // -Y  
			pointlight.LightSpaceMatrix[4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));   // +Z
			pointlight.LightSpaceMatrix[5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));  // -Z);

			s_Data.PointLightBuffer.push_back(pointlight);
		}
	}

	void SceneRenderer::InitRenderGraph()
	{
		// 几何Pass
		{
			FramebufferSpecification geoFramebufferSpec;
			geoFramebufferSpec.Width = 1280;
			geoFramebufferSpec.Height = 720;
			geoFramebufferSpec.Attachments = {
				FramebufferTextureFormat::RGBA8,       // 0: Albedo (RGB) + Alpha (A)
				FramebufferTextureFormat::RED_INTEGER, // 1: EntityID
				FramebufferTextureFormat::RGBA8,     // 2: Normal (世界空间)
				FramebufferTextureFormat::RGBA8,       // 3: Metallic, Roughness, AO
				FramebufferTextureFormat::Depth,        // 4: Depth
			};
			geoFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

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
		}

		// 阴影Pass
		{
			FramebufferSpecification shadowFramebufferSpec;
			shadowFramebufferSpec.Width = 2048;
			shadowFramebufferSpec.Height = 2048;
			shadowFramebufferSpec.Attachments = { FramebufferTextureFormat::Depth };
			shadowFramebufferSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			RenderPassSpecification shadowPassSpec;
			shadowPassSpec.TargetFramebuffer = Framebuffer::Create(shadowFramebufferSpec);
			shadowPassSpec.m_Shader = Shader::Create("assets/shaders/shadow_pass.glsl");
			auto shadowPass = RenderPass::Create(shadowPassSpec);

			AddRenderNode("Shadow", shadowPass, [shadowPass]() {
				auto framebuffer = shadowPass->GetSpecification().TargetFramebuffer;
				auto shader = shadowPass->GetSpecification().m_Shader;

				std::vector<std::pair<glm::mat4, Ref<Mesh>>> shadowEntities;
				if (s_Data.ActiveScene) {
					auto view = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, MeshComponent>();
					shadowEntities.reserve(view.size_hint());
					for (auto entity : view) {
						// 直接解包元组并构造pair
						auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);
						shadowEntities.emplace_back(transform.GetTransform(), mesh.mesh);
					}
				}

				// 渲染方向光阴影（CSM）
				shader->SetInt("u_LightType", 0);
				for (size_t i = 0; i < s_Data.DirLightBuffer.size(); ++i) {
					// 绑定当前光源的阴影纹理层到帧缓冲
					auto& dirShadowMap = s_Data.DirShadowMapArray;

					shader->SetInt("u_LightIndex", i);

					framebuffer->AttachTextureLayer(dirShadowMap, 0, i);

					// 清除深度缓冲
					RenderCommand::Clear();

					// 渲染场景中所有物体到当前阴影贴图
					for (auto& [transform, mesh] : shadowEntities) {
						Renderer::SubmitMeshShadow(mesh, transform);
					}

					//// 为每个级联阴影附着对应的纹理层（假设每个方向光有4个级联）
					//for (uint32_t cascade = 0; cascade < 1; ++cascade) {
					//	uint32_t layer = i * 4 + cascade; // 计算层索引（方向光索引*4 + 级联索引）
					//	framebuffer->AttachTextureLayer(dirShadowMap, 0, layer);

					//	shader->SetInt("u_TexIndex", cascade);

					//	// 清除深度缓冲
					//	RenderCommand::Clear();

					//	// 渲染场景中所有物体到当前阴影贴图
					//	for (auto& [transform, mesh] : shadowEntities) {
					//		Renderer::SubmitMeshShadow(mesh, transform);
					//	}
					//}
				}

				// 渲染点光源阴影
				shader->SetInt("u_LightType", 1);
				for (size_t i = 0; i < s_Data.PointLightBuffer.size(); ++i) {
					auto& pointShadowMap = s_Data.PointShadowMapArray;

					shader->SetInt("u_LightIndex", i);

					// 点光源有6个面（立方体贴图）
					const std::vector<uint32_t> cubeFaces = {
						0, // GL_TEXTURE_CUBE_MAP_POSITIVE_X
						1, // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
						2, // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
						3, // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
						4, // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
						5  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
					};

					// 为每个面附着对应的立方体贴图层
					for (uint32_t face : cubeFaces) {
						framebuffer->AttachCubeMapLayer(pointShadowMap, 0, face, i);

						shader->SetInt("u_TexIndex", face);

						// 清除深度缓冲
						RenderCommand::Clear();

						// 渲染场景中所有物体到当前阴影贴图
						for (auto& [transform, mesh] : shadowEntities) {
							Renderer::SubmitMeshShadow(mesh, transform);
						}
					}
				}
				}, {}, { "DirShadowMap", "PointShadowMap" });
		}

		// 光照Pass
		{
			FramebufferSpecification lightingFramebufferSpec;
			lightingFramebufferSpec.Width = 1280;
			lightingFramebufferSpec.Height = 720;
			lightingFramebufferSpec.Attachments = {
				FramebufferTextureFormat::RGBA16F  // 0: Lighting Result
			};

			RenderPassSpecification lightingRenderPassSpec;
			lightingRenderPassSpec.TargetFramebuffer = Framebuffer::Create(lightingFramebufferSpec);
			lightingRenderPassSpec.m_Shader = Shader::Create("assets/shaders/phong_pass.glsl");
			auto lightingPass = RenderPass::Create(lightingRenderPassSpec);
			lightingPass->SetUniform("u_EnvMap", s_Data.EnvMap);

			AddRenderNode("DeferredLighting", lightingPass, [lightingPass]() {
				Renderer::SubmitFullscreenQuad();
				}, { "GBufferAlbedo", "GBufferNormal", "GBufferMP", "GBufferDepth", "DirShadowMap", "PointShadowMap" }, { "LightingResult" });
		}
		
		// 混合Pass
		{
			FramebufferSpecification comFramebufferSpec;
			comFramebufferSpec.Width = 1280;
			comFramebufferSpec.Height = 720;
			comFramebufferSpec.Attachments = {
				FramebufferTextureFormat::RGBA8
			};
			comFramebufferSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

			// Composite Render Pass Specification
			RenderPassSpecification comRenderPassSpec;
			comRenderPassSpec.TargetFramebuffer = Framebuffer::Create(comFramebufferSpec);
			comRenderPassSpec.m_Shader = Shader::Create("assets/shaders/hdr.glsl");
			auto compPass = RenderPass::Create(comRenderPassSpec);
			compPass->SetUniform("u_Exposure", s_Data.Exposure);

			AddRenderNode("Composite", compPass, [compPass]() {
				Renderer::SubmitFullscreenQuad();
				}, { "LightingResult" }, { "FinalColor" });
		}

		//阴影图测试Pass
		{
			FramebufferSpecification testSpec;
			testSpec.Width = 1280;
			testSpec.Height = 720;
			testSpec.Attachments = {
				FramebufferTextureFormat::RGBA16F  // 0: Lighting Result
			};

			RenderPassSpecification testPassSpec;
			testPassSpec.TargetFramebuffer = Framebuffer::Create(testSpec);
			testPassSpec.m_Shader = Shader::Create("assets/shaders/shadowmap_test.glsl");
			auto shadowmaptestPass = RenderPass::Create(testPassSpec);

			AddRenderNode("Shadow_Map_Test", shadowmaptestPass, [shadowmaptestPass]() {
				Renderer::SubmitFullscreenQuad();
				}, { "DirShadowMap", "PointShadowMap", "FinalColor" }, { "ShadowMap" });
		}
	}

	void SceneRenderer::AddRenderNode(const std::string& name, Ref<RenderPass> pass,
		std::function<void()> executeFunc,
		const std::vector<std::string>& inputs,
		const std::vector<std::string>& outputs,
		const int executeInterval) {
		s_Data.RenderNodes.push_back({ name, pass, executeFunc, inputs, outputs, executeInterval });
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
				} else if(node.outputs[i].find("DirShadowMap") != std::string::npos){
					s_Data.GraphResources[node.outputs[i]] = s_Data.DirShadowMapArray;
				} else if (node.outputs[i].find("PointShadowMap") != std::string::npos) {
					s_Data.GraphResources[node.outputs[i]] = s_Data.PointShadowMapArray;
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

			// 检查执行间隔
			if (node.shouldExecute()) continue;

			// 自动绑定输入纹理
			BindNodeInputs(node);

			Renderer::BeginRenderPass(node.pass);

			auto specofocatopm = node.pass->GetSpecification().TargetFramebuffer->GetSpecification();
			if (node.clearOnExecute) {
				//RenderCommand::SetViewport(0, 0, specofocatopm.Width, specofocatopm.Height);
				RenderCommand::SetClearColor(specofocatopm.ClearColor);
				RenderCommand::Clear();
			}

			//全局参数
			s_Data.CameraUniformBuffer->Bind();

			s_Data.DirLightUniformBuffer->SetData(
				s_Data.DirLightBuffer.empty() ? nullptr : s_Data.DirLightBuffer.data(),
				s_Data.DirLightBuffer.empty() ? 0 : sizeof(DirLight) * s_Data.DirLightBuffer.size()
			);
			s_Data.DirLightUniformBuffer->Bind();

			s_Data.PointLightUniformBuffer->SetData(
				s_Data.PointLightBuffer.empty() ? nullptr : s_Data.PointLightBuffer.data(),
				s_Data.PointLightBuffer.empty() ? 0 : sizeof(PointLight) * s_Data.PointLightBuffer.size()
			);
			s_Data.PointLightUniformBuffer->Bind();

			// 执行渲染逻辑
			node.executeFunction();

			Renderer::EndRenderPass();

			// 更新输出资源映射（处理可能的附件变化）
			UpdateNodeOutputs(node);
		}
	}

	void SceneRenderer::BindNodeInputs(const RenderGraphNode& node)
	{
		static Ref<Texture2D> defaultTexture = Texture2D::Create("assets/textures/defaultTexture.jpg");

		for (size_t textureUnit = 0; textureUnit < node.inputs.size(); ++textureUnit) {
			if(textureUnit > 8)
				AF_CORE_WARN("RenderGraph: Input resource 数量为 '{}'， 超出上限8", textureUnit);
			const auto& inputName = node.inputs[textureUnit];
			auto it = s_Data.GraphResources.find(inputName);
			if (it != s_Data.GraphResources.end() && it->second) {
				node.pass->SetUniform(inputName, it->second);
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

	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		for (auto& node : s_Data.RenderNodes) {
			auto& framebuffer = node.pass->GetSpecification().TargetFramebuffer;

			// 跳过阴影贴图相关的帧缓冲（根据名称判断）
			if (node.name == "Shadow") {
				continue; // 阴影贴图尺寸保持初始化时的设置（如2048x2048）
			}

			// 其他帧缓冲（如Geometry、DeferredLighting、Composite）按窗口尺寸调整
			framebuffer->Resize(width, height);
		}

		// 视口改变后需要更新所有资源引用
		UpdateGraphResources();
	}

	Ref<Texture2D> SceneRenderer::GetFinalColorBuffer()
	{
		auto it = s_Data.GraphResources.find("FinalColor");
		if (it != s_Data.GraphResources.end() && it->second) {
			return std::dynamic_pointer_cast<Texture2D>(it->second);
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