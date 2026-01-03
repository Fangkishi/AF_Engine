#include "afpch.h"
#include "SceneRenderer.h"
#include "AF/Renderer/Renderer.h"
#include "AF/Renderer/Renderer2D.h"
#include "AF/Renderer/RenderPass.h"

#include <queue>
#include <glad/glad.h>

namespace AF {

	// 初始化静态全局数据实例
	SceneRenderer::SceneRendererData SceneRenderer::s_Data;

	// ===================================================================================
	// --- 核心生命周期与基础接口 (Lifecycle) ---
	// ===================================================================================

	/**
	 * @brief 初始化渲染系统
	 * 1. 分配 Uniform Buffer (UBO) 用于同步相机矩阵。
	 * 2. 分配 Shader Storage Buffer (SSBO) 用于动态光源列表。
	 * 3. 创建阴影贴图所需的纹理数组 (Texture Array / Cube Array)。
	 * 4. 载入环境贴图并构建初始渲染图。
	 */
	void SceneRenderer::Init()
	{
		// 1. 初始化相机 UBO (binding = 0)
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(s_Data.CameraBuffer), 0);

		// 2. 初始化光源 SSBO (预留 4 个平行光，16 个点光源空间)
		s_Data.DirLightBuffer.reserve(4); 
		s_Data.PointLightBuffer.reserve(16);
		s_Data.DirLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(DirLight) * s_Data.DirLightBuffer.capacity(), 0);
		s_Data.PointLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(PointLight) * s_Data.PointLightBuffer.capacity(), 1);

		// 3. 初始化阴影贴图资产
		TextureSpecification dirShadowMapSpec;
		dirShadowMapSpec.Width = 2048;
		dirShadowMapSpec.Height = 2048;
		dirShadowMapSpec.Format = ImageFormat::DEPTH24STENCIL8;
		dirShadowMapSpec.ArraySize = 4; // 支持多光源或级联
		s_Data.DirShadowMapArray = Texture2D::Create(dirShadowMapSpec);

		TextureSpecification pointShadowMapSpec;
		pointShadowMapSpec.Width = 2048;
		pointShadowMapSpec.Height = 2048;
		pointShadowMapSpec.Format = ImageFormat::DEPTH24STENCIL8;
		pointShadowMapSpec.ArraySize = 16; // 支持多个点光源立方体贴图
		s_Data.PointShadowMapArray = TextureCube::Create(pointShadowMapSpec);

		// 4. 环境与渲染图初始化
		s_Data.EnvMap = Texture2D::Create("assets/textures/bk.jpg");
		InitRenderGraph();
	}

	/**
	 * @brief 视口同步
	 * 遍历所有渲染节点，调整其 Framebuffer 尺寸，并刷新全局资源引用。
	 */
	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		for (auto& node : s_Data.RenderNodes) {
			auto& framebuffer = node.pass->GetSpecification().TargetFramebuffer;

			// 阴影图通常是固定高分辨率，不随视口实时缩放
			if (node.name == "Shadow") continue; 

			framebuffer->Resize(width, height);
		}

		// 同步资源池中的纹理引用
		UpdateGraphResources();
	}

	// ===================================================================================
	// --- 场景提交与控制流程 (Flow Control) ---
	// ===================================================================================

	/**
	 * @brief 准备渲染当前场景
	 * - 提取相机组件并同步 View/Projection 矩阵。
	 * - 触发光源收集流程。
	 */
	void SceneRenderer::BeginScene(const Ref<Scene>& scene)
	{
		s_Data.ActiveScene = scene;

		// 同步相机 UBO
		auto camera = s_Data.ActiveScene->GetCamera();
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 proj = camera->GetProjection();

		s_Data.CameraBuffer.ViewPosition = camera->GetPosition();
		s_Data.CameraBuffer.View = view;
		s_Data.CameraBuffer.ViewInverse = glm::inverse(view);
		s_Data.CameraBuffer.Projection = proj;
		s_Data.CameraBuffer.ProjectionInverse = glm::inverse(proj);
		
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(s_Data.CameraBuffer));

		// 提取光源数据
		CollectSceneLights();
	}

	/**
	 * @brief 结束渲染
	 * - 重新编译渲染图（处理动态节点变更）。
	 * - 执行 GPU 渲染序列。
	 */
	void SceneRenderer::EndScene()
	{
		CompileRenderGraph(); 
		ExecuteRenderGraph(); 
		s_Data.ActiveScene = nullptr;
	}

	// ===================================================================================
	// --- 公共 API 与资源查询 ---
	// ===================================================================================

	/**
	 * @brief 注册节点到渲染图中
	 */
	void SceneRenderer::AddRenderNode(const std::string& name, Ref<RenderPass> pass,
		std::function<void()> executeFunc,
		const std::vector<std::string>& inputs,
		const std::vector<std::string>& outputs,
		const int executeInterval) {
		s_Data.RenderNodes.push_back({ name, pass, executeFunc, inputs, outputs, executeInterval });
	}

	/**
	 * @brief 获取渲染图中指定名称的资源纹理
	 */
	Ref<Texture2D> SceneRenderer::GetBuffer(const std::string& name)
	{
		auto it = s_Data.GraphResources.find(name);
		if (it != s_Data.GraphResources.end() && it->second) {
			return std::dynamic_pointer_cast<Texture2D>(it->second);
		}
		AF_CORE_ERROR("RenderGraph: 资源 {0} 未找到!", name);
		return nullptr;
	}

	/**
	 * @brief 获取指定资源的 OpenGL Renderer ID
	 */
	uint32_t SceneRenderer::GetBufferRendererID(const std::string& name)
	{
		auto tex = GetBuffer(name);
		return tex ? tex->GetRendererID() : 0;
	}

	/**
	 * @brief 读取特定坐标的像素数据（常用于物体拾取）
	 */
	int SceneRenderer::ReadPixel(int x, int y)
	{
		for (auto& node : s_Data.RenderNodes) {
			if (node.name == "Geometry") {
				auto fb = node.pass->GetSpecification().TargetFramebuffer;
				fb->Bind();
				int pixel = fb->ReadPixel(1, x, y); // 通常 1 号附件是 EntityID
				fb->Unbind();
				return pixel;
			}
		}
		return -1;
	}

	// ===================================================================================
	// --- 渲染图 (Render Graph) 架构定义 ---
	// ===================================================================================

	/**
	 * @brief 构建默认的延迟渲染管线
	 * 
	 * 1. **Geometry Pass (G-Buffer)**:
	 *    - 将场景所有不透明物体的颜色、法线、材质参数 and EntityID 渲染到多个附件。
	 *    - 实现：遍历 ECS，按材质排序以减少状态切换。
	 * 
	 * 2. **Shadow Pass (Shadow Maps)**:
	 *    - 为平行光和点光源生成深度贴图。
	 *    - 实现：切换 Framebuffer 到纹理数组的不同层进行多次渲染。
	 * 
	 * 3. **Deferred Lighting Pass**:
	 *    - 读取 G-Buffer 和 Shadow Maps，计算最终物理光照。
	 *    - 实现：全屏四边形绘制，触发片段着色器计算。
	 * 
	 * 4. **Composite Pass**:
	 *    - 进行 HDR Tone Mapping, Gamma 校正。
	 *    - 实现：全屏四边形绘制。
	 */
	void SceneRenderer::InitRenderGraph()
	{
		// -----------------------------------------------------------------------------------
		// 1. 几何阶段 (Geometry Pass)
		// -----------------------------------------------------------------------------------
		{
			FramebufferSpecification spec;
			spec.Attachments = {
				FramebufferTextureFormat::RGBA8,       // Color (Albedo)
				FramebufferTextureFormat::RED_INTEGER, // EntityID (Picking)
				FramebufferTextureFormat::RGBA8,       // Normal (World Space)
				FramebufferTextureFormat::RGBA8,       // Metallic / Roughness / AO
				FramebufferTextureFormat::DEPTH24STENCIL8 // Depth
			};
			auto pass = RenderPass::Create({ Framebuffer::Create(spec), Shader::Create("assets/shaders/geometry_pass.glsl") });

			AddRenderNode("Geometry", pass, [pass]() {
				pass->GetSpecification().TargetFramebuffer->ClearAttachment(1, -1);
				
				if (s_Data.ActiveScene) {
					auto view = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, MeshComponent, MaterialComponent>();
					
					// 性能优化：按材质进行 CPU 排序，减少 GPU 状态切换开销
					std::vector<entt::entity> entities;
					entities.reserve(view.size_hint());
					for (auto entity : view) entities.push_back(entity);

					std::sort(entities.begin(), entities.end(), [&view](entt::entity lhs, entt::entity rhs) {
						auto& lhsMat = view.get<MaterialComponent>(lhs).material;
						auto& rhsMat = view.get<MaterialComponent>(rhs).material;
						if (lhsMat != rhsMat) return lhsMat < rhsMat;

						return view.get<MeshComponent>(lhs).mesh < view.get<MeshComponent>(rhs).mesh;
					});

					for (auto entity : entities) {
						auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);
						transform.UpdateTransform();
						Renderer::SubmitMesh(mesh.mesh, material.material, mesh.instanceUniforms);
					}
				}
			}, {}, { "GBufferAlbedo", "GBufferEntityID", "GBufferNormal", "GBufferMP", "GBufferDepth" });
		}

		// -----------------------------------------------------------------------------------
		// 2. 阴影阶段 (Shadow Pass)
		// -----------------------------------------------------------------------------------
		{
			FramebufferSpecification spec;
			spec.Width = 2048; spec.Height = 2048;
			spec.Attachments = { FramebufferTextureFormat::DEPTH };
			auto pass = RenderPass::Create({ Framebuffer::Create(spec), Shader::Create("assets/shaders/shadow_pass.glsl") });

			AddRenderNode("Shadow", pass, [pass]() {
				auto shader = pass->GetSpecification().m_Shader;
				auto fb = pass->GetSpecification().TargetFramebuffer;
				
				auto view = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, MeshComponent>();

				std::vector<entt::entity> entities;
				entities.reserve(view.size_hint());
				for (auto entity : view) entities.push_back(entity);

				std::sort(entities.begin(), entities.end(), [&view](entt::entity lhs, entt::entity rhs) {
					return view.get<MeshComponent>(lhs).mesh < view.get<MeshComponent>(rhs).mesh;
				});

				// --- 方向光阴影 ---
				shader->SetInt("u_LightType", 0);
				fb->AttachTexture(s_Data.DirShadowMapArray, 0);
				RenderCommand::Clear();

				const int dirBatchSize = 32;
				int dirLightCount = (int)s_Data.DirLightBuffer.size();

				if (dirLightCount > 0)
				{
					for (int i = 0; i < dirLightCount; i += dirBatchSize) {
						int count = std::min(dirBatchSize, dirLightCount - i);
						shader->SetInt("u_LightStart", i);
						shader->SetInt("u_LightCount", count);

						for (auto entity : entities) {
							auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);
							transform.UpdateTransform();
							Renderer::SubmitShadow(mesh.mesh, mesh.instanceUniforms);
						}
					}
				}

				// --- 点光源阴影 ---
				shader->SetInt("u_LightType", 1);
				fb->AttachTexture(s_Data.PointShadowMapArray, 0);
				RenderCommand::Clear();

				const int pointBatchSize = 5;
				int pointLightCount = (int)s_Data.PointLightBuffer.size();

				if (pointLightCount > 0)
				{
					for (int i = 0; i < pointLightCount; i += pointBatchSize) {
						int count = std::min(pointBatchSize, pointLightCount - i);
						shader->SetInt("u_LightStart", i);
						shader->SetInt("u_LightCount", count);

						for (auto entity : entities) {
							auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);
							transform.UpdateTransform();
							Renderer::SubmitShadow(mesh.mesh, mesh.instanceUniforms);
						}
					}
				}
			}, {}, { "DirShadowMap", "PointShadowMap" });
		}

		// -----------------------------------------------------------------------------------
		// 3. 延迟光照阶段 (Lighting Pass)
		// -----------------------------------------------------------------------------------
		{
			FramebufferSpecification spec;
			spec.Attachments = { FramebufferTextureFormat::RGBA16F }; // HDR Support
			auto pass = RenderPass::Create({ Framebuffer::Create(spec), Shader::Create("assets/shaders/phong_pass.glsl") });
			pass->SetUniform("u_EnvMap", s_Data.EnvMap);

			AddRenderNode("DeferredLighting", pass, []() {
				Renderer::SubmitFullscreenQuad();
			}, { "GBufferAlbedo", "GBufferNormal", "GBufferMP", "GBufferDepth", "DirShadowMap", "PointShadowMap" }, { "LightingResult" });
		}

		// -----------------------------------------------------------------------------------
		// 4. 后处理与混合 (PostProcess Pass)
		// -----------------------------------------------------------------------------------
		{
			FramebufferSpecification spec;
			spec.Attachments = { FramebufferTextureFormat::RGBA8 };
			auto pass = RenderPass::Create({ Framebuffer::Create(spec), Shader::Create("assets/shaders/hdr.glsl") });
			
			AddRenderNode("Composite", pass, []() {
				Renderer::SubmitFullscreenQuad();
			}, { "LightingResult" }, { "FinalColor" });
		}
	}

	/**
	 * @brief 编译渲染图
	 * 采用 Kahn's Algorithm 进行拓扑排序：
	 * 1. 识别资源生产者与消费者。
	 * 2. 构建节点入度表。
	 * 3. 将入度为 0 的节点加入执行序列，并递归减少其依赖项的入度。
	 * 4. 最终生成一个线性的、满足依赖关系的执行顺序 ExecutionOrder。
	 */
	void SceneRenderer::CompileRenderGraph()
	{
		s_Data.ResourceProducers.clear();
		s_Data.ResourceConsumers.clear();
		s_Data.ExecutionOrder.clear();

		// 1. 建立资源映射
		for (const auto& node : s_Data.RenderNodes) {
			for (const auto& out : node.outputs) s_Data.ResourceProducers[out].push_back(node.name);
			for (const auto& in : node.inputs) s_Data.ResourceConsumers[in].push_back(node.name);
		}

		// 2. 初始化入度表
		std::unordered_map<std::string, int> inDegree;
		std::unordered_map<std::string, std::vector<std::string>> adj;
		for (const auto& node : s_Data.RenderNodes) inDegree[node.name] = 0;

		// 3. 构建邻接矩阵与入度
		for (const auto& node : s_Data.RenderNodes) {
			for (const auto& in : node.inputs) {
				for (const auto& prod : s_Data.ResourceProducers[in]) {
					if (prod != node.name) {
						adj[prod].push_back(node.name);
						inDegree[node.name]++;
					}
				}
			}
		}

		// 4. 拓扑排序
		std::queue<std::string> q;
		for (auto& [name, degree] : inDegree) if (degree == 0) q.push(name);

		while (!q.empty()) {
			std::string curr = q.front(); q.pop();
			s_Data.ExecutionOrder.push_back(curr);
			for (auto& next : adj[curr]) {
				if (--inDegree[next] == 0) q.push(next);
			}
		}

		s_Data.GraphCompiled = true;
		UpdateGraphResources();
	}

	/**
	 * @brief 执行渲染图
	 * 按 ExecutionOrder 循环执行节点逻辑，自动处理 UBO/SSBO 的绑定与更新。
	 */
	void SceneRenderer::ExecuteRenderGraph()
	{
		if (!s_Data.GraphCompiled) return;

		UpdateGraphResources();

		for (const auto& nodeName : s_Data.ExecutionOrder) {
			auto it = std::find_if(s_Data.RenderNodes.begin(), s_Data.RenderNodes.end(), 
				[&](const RenderGraphNode& n) { return n.name == nodeName; });
			if (it == s_Data.RenderNodes.end()) continue;

			auto& node = *it;
			if (node.shouldExecute()) continue;

			// --- 执行前准备 ---
			BindNodeInputs(node);              
			Renderer::BeginRenderPass(node.pass); 

			if (node.clearOnExecute) {
				RenderCommand::SetClearColor(node.pass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor);
				RenderCommand::Clear();
			}

			// --- 同步全局 GPU 缓冲区 ---
			s_Data.CameraUniformBuffer->Bind();
			
			// 上传方向光 SSBO (binding = 0)
			if (s_Data.DirLightBuffer.empty()) s_Data.DirLightUniformBuffer->SetData(nullptr, 0);
			else s_Data.DirLightUniformBuffer->SetData(s_Data.DirLightBuffer.data(), (uint32_t)(sizeof(DirLight) * s_Data.DirLightBuffer.size()));
			s_Data.DirLightUniformBuffer->Bind();
			
			// 上传点光源 SSBO (binding = 1)
			if (s_Data.PointLightBuffer.empty()) s_Data.PointLightUniformBuffer->SetData(nullptr, 0);
			else s_Data.PointLightUniformBuffer->SetData(s_Data.PointLightBuffer.data(), (uint32_t)(sizeof(PointLight) * s_Data.PointLightBuffer.size()));
			s_Data.PointLightUniformBuffer->Bind();

			// --- 执行节点渲染逻辑 ---
			node.executeFunction();

			// --- 结束当前阶段 ---
			Renderer::EndRenderPass();
			UpdateNodeOutputs(node);
		}
	}

	// ===================================================================================
	// --- 内部资源管理辅助 ---
	// ===================================================================================

	/** @brief 检查格式是否为深度格式 */
	static bool IsDepthFormat(FramebufferTextureFormat format) {
		return format == FramebufferTextureFormat::DEPTH24STENCIL8 || format == FramebufferTextureFormat::DEPTH;
	}

	/** @brief 同步所有节点资源池状态 */
	void SceneRenderer::UpdateGraphResources() {
		for (const auto& node : s_Data.RenderNodes) UpdateNodeOutputs(node);
	}

	/** @brief 绑定节点所需的纹理资源 */
	void SceneRenderer::BindNodeInputs(const RenderGraphNode& node) {
		for (const auto& in : node.inputs) {
			auto it = s_Data.GraphResources.find(in);
			if (it != s_Data.GraphResources.end() && it->second) node.pass->SetUniform(in, it->second);
		}
	}

	/** @brief 将节点输出的附件注册到全局资源名下 */
	void SceneRenderer::UpdateNodeOutputs(const RenderGraphNode& node) {
		auto fb = node.pass->GetSpecification().TargetFramebuffer;
		auto& attachments = fb->GetSpecification().Attachments.Attachments;
		uint32_t colorIdx = 0;

		for (size_t i = 0; i < node.outputs.size(); i++) {
			auto& name = node.outputs[i];
			if (name == "DirShadowMap") { s_Data.GraphResources[name] = s_Data.DirShadowMapArray; continue; }
			if (name == "PointShadowMap") { s_Data.GraphResources[name] = s_Data.PointShadowMapArray; continue; }

			if (i < attachments.size()) {
				if (IsDepthFormat(attachments[i].TextureFormat)) s_Data.GraphResources[name] = fb->GetDepthAttachment();
				else s_Data.GraphResources[name] = fb->GetColorAttachment(colorIdx++);
			}
		}
	}

	// ===================================================================================
	// --- 场景数据预处理 (Preprocessing) ---
	// ===================================================================================

	/**
	 * @brief 光源收集与空间矩阵计算
	 * 1. 遍历场景提取所有活跃的光源组件。
	 * 2. 为每个光源计算阴影变换矩阵。
	 *    - 平行光：使用正交投影，围绕相机位置构建阴影盒。
	 *    - 点光源：使用透视投影，为 6 个面分别构建视图矩阵。
	 */
	void SceneRenderer::CollectSceneLights()
	{
		s_Data.DirLightBuffer.clear();
		s_Data.PointLightBuffer.clear();

		// 1. 处理方向光
		auto dView = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, DirectionalLightComponent>();
		for (auto e : dView) {
			auto [transform, light] = dView.get<TransformComponent, DirectionalLightComponent>(e);
			if (!light.Enabled) continue;

			DirLight dl;
			dl.direction = glm::normalize(glm::rotate(glm::quat(transform.Rotation), glm::vec3(0, 0, -1)));
			dl.ambient = light.Ambient; dl.diffuse = light.Diffuse; dl.specular = light.Specular;
			
			// 计算正交投影阴影盒
			glm::vec3 camPos = s_Data.ActiveScene->GetCamera()->GetPosition();
			glm::mat4 lightView = glm::lookAt(camPos - dl.direction * 50.0f, camPos, {0, 1, 0});
			dl.LightSpaceMatrix = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 0.1f, 100.0f) * lightView;
			
			s_Data.DirLightBuffer.push_back(dl);
		}

		// 2. 处理点光源
		auto pView = s_Data.ActiveScene->GetAllEntitiesWithView<TransformComponent, PointLightComponent>();
		for (auto e : pView) {
			auto [transform, light] = pView.get<TransformComponent, PointLightComponent>(e);
			if (!light.Enabled) continue;

			PointLight pl;
			pl.position = transform.Translation; pl.color = light.Color; pl.intensity = light.Intensity;
			
			// 计算点光源 6 个面的透视投影矩阵
			glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
			
			// 为立方体贴图的 6 个面分别生成视图矩阵
			// 注意：OpenGL 的立方体贴图坐标系要求特定的 Up 向量
			pl.LightSpaceMatrix[0] = proj * glm::lookAt(pl.position, pl.position + glm::vec3(1, 0, 0), {0, -1, 0});  // +X
			pl.LightSpaceMatrix[1] = proj * glm::lookAt(pl.position, pl.position + glm::vec3(-1, 0, 0), {0, -1, 0}); // -X
			pl.LightSpaceMatrix[2] = proj * glm::lookAt(pl.position, pl.position + glm::vec3(0, 1, 0), {0, 0, 1});   // +Y
			pl.LightSpaceMatrix[3] = proj * glm::lookAt(pl.position, pl.position + glm::vec3(0, -1, 0), {0, 0, -1}); // -Y
			pl.LightSpaceMatrix[4] = proj * glm::lookAt(pl.position, pl.position + glm::vec3(0, 0, 1), {0, -1, 0});  // +Z
			pl.LightSpaceMatrix[5] = proj * glm::lookAt(pl.position, pl.position + glm::vec3(0, 0, -1), {0, -1, 0}); // -Z
			
			s_Data.PointLightBuffer.push_back(pl);
		}
	}
}
