#include "afpch.h"
#include "Renderer.h"
#include "AF/Renderer/Renderer2D.h"
#include "AF/Renderer/SceneRenderer.h"
#include "AF/Renderer/RenderCommand.h"

#include <glad/glad.h>

namespace AF {

	/**
	 * @struct RendererData
	 * @brief 存储渲染器全局状态的内部容器
	 */
	struct RendererData
	{
		// --- 当前活跃句柄 (Current State) ---
		Ref<RenderPass> m_ActiveRenderPass;           ///< 当前正在执行的渲染通道
		Ref<VertexArray> m_FullscreenQuadVertexArray; ///< 预置的全屏绘制几何数据 (NDC Quad)

		// --- 系统默认资源 (Fallback Resources) ---
		Ref<Material> DefaultMaterial;                ///< 全局兜底材质，防止未分配材质的物体导致崩溃
		Ref<Shader> DefaultShader;                    ///< 全局兜底着色器

		// --- 状态追踪与性能优化 (State Tracking) ---
		int pipelineUnit = 0;                         ///< 当前 Pass 级别的纹理槽位偏移量
		int materialUnit = 0;                         ///< 当前材质级别的纹理槽位偏移量
		TextureUnitCache textureUnitCache;            ///< 纹理槽位缓存，用于在同一 Draw Call 内去重纹理绑定

		Ref<Mesh> m_ActiveMesh = nullptr;             ///< 缓存上一次渲染的网格，用于减少冗余的 VAO 切换与 Uniform 应用
		Ref<Material> m_ActiveMaterial = nullptr;     ///< 缓存上一次渲染的材质，用于减少冗余的 Shader 参数更新
	};

	static RendererData s_Data; // 内部全局单例数据

	// ===================================================================================
	// --- 基础生命周期管理 ---
	// ===================================================================================

	/**
	 * @brief 初始化渲染系统
	 * 1. 启动 RenderCommand, Renderer2D, SceneRenderer 等核心模块。
	 * 2. 构建引擎默认材质，预设 Albedo, Metallic, Roughness 等 PBR 参数。
	 * 3. 预构建 NDC 空间的全屏四边形顶点数据，供延迟渲染和后处理使用。
	 */
	void Renderer::Init()
	{
		AF_PROFILE_FUNCTION();

		// 1. 初始化核心渲染模块
		RenderCommand::Init();
		Renderer2D::Init();
		SceneRenderer::Init();

		// 2. 配置默认 PBR 材质参数 (兜底方案)
		s_Data.DefaultMaterial = Material::CreatePBR();

		// 3. 初始默认着色器
		s_Data.DefaultShader = Shader::Create("assets/shaders/phong.glsl");

		// 4. 构建全屏绘制几何体 (NDC Quad: -1 to 1)
		s_Data.m_FullscreenQuadVertexArray = VertexArray::Create();

		float vertices[] = {
			// 位置 (x, y, z)     // UV (u, v)
			-1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,   0.0f, 1.0f,

			 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f,   0.0f, 1.0f
		};

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		vertexBuffer->SetLayout(layout);
		s_Data.m_FullscreenQuadVertexArray->AddVertexBuffer(vertexBuffer);
	}

	void Renderer::Shutdown()
	{
		// 清理逻辑待扩展
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	// ===================================================================================
	// --- 渲染通道 (Render Pass) 流程控制 ---
	// ===================================================================================

	/**
	 * @brief 开启渲染阶段
	 * 职责：
	 * 1. 切换 GPU 渲染目标 (Framebuffer)。
	 * 2. 绑定当前 Pass 对应的 Shader。
	 * 3. 重置纹理槽位和状态追踪缓存，确保当前 Pass 的状态独立。
	 * 4. 应用属于整个 Pass 的全局参数（如投影矩阵、环境光）。
	 */
	void Renderer::BeginRenderPass(const Ref<RenderPass> renderPass)
	{
		s_Data.m_ActiveRenderPass = renderPass;

		// 1. 激活渲染目标
		renderPass->GetSpecification().TargetFramebuffer->Bind();

		// 2. 确定并激活 Shader
		Ref<Shader> shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;
		if (!shader) shader = s_Data.DefaultShader;
		shader->Bind();

		// 3. 状态重置：为当前 Pass 的第一次绘制做准备
		s_Data.pipelineUnit = 0;
		s_Data.materialUnit = 0;
		s_Data.textureUnitCache.clear();
		s_Data.m_ActiveMesh = nullptr;
		s_Data.m_ActiveMaterial = nullptr;

		// 4. 应用全局 Pass Uniform (isGlobal=true)
		renderPass->Apply(shader, true, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
	}

	void Renderer::EndRenderPass()
	{
		// 解绑当前渲染目标
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();

		// 重置纹理计数，防止跨通道纹理污染
		s_Data.pipelineUnit = 0;
		s_Data.materialUnit = 0;
	}

	// ===================================================================================
	// --- 几何体提交 (Drawing Submission) ---
	// ===================================================================================

	/**
	 * @brief 执行网格渲染
	 * 通过“状态追踪”技术优化性能：
	 * - 如果连续绘制使用相同材质的物体，跳过材质级 Uniform 的重新绑定。
	 * - 如果连续绘制相同网格，跳过网格级 Uniform 的重新绑定。
	 */
	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const UniformContainer& instanceUniforms)
	{
		auto& material = overridematerial ? overridematerial : s_Data.DefaultMaterial;
		
		Ref<Shader> shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;
		if (!shader) shader = s_Data.DefaultShader;

		// 1. 应用材质参数 (仅在材质实例切换时执行)
		if (s_Data.m_ActiveMaterial != material) {
			s_Data.materialUnit = 0; // 每个材质从 0 号材质纹理单元开始计数
			material->Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
			s_Data.m_ActiveMaterial = material;
		}

		// 2. 应用网格参数 (仅在网格资源切换时执行)
		if (s_Data.m_ActiveMesh != mesh) {
			mesh->Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
			s_Data.m_ActiveMesh = mesh;
		}

		// 3. 应用实例私有参数 (如变换矩阵，必须每次更新)
		instanceUniforms.Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);

		// 4. 派发 Draw Call
		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	/**
	 * @brief 提交阴影绘制 (精简版 SubmitMesh)
	 * 仅处理网格几何信息和位置变换，忽略光照、颜色等材质参数，极大减少阴影渲染阶段的 CPU/GPU 开销。
	 */
	void Renderer::SubmitShadow(const Ref<Mesh>& mesh, const UniformContainer& instanceUniforms)
	{
		Ref<Shader> shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;
		if (!shader) shader = s_Data.DefaultShader;

		s_Data.materialUnit = 0;
		
		// 1. 网格切换检查
		if (s_Data.m_ActiveMesh != mesh) {
			mesh->Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
			s_Data.m_ActiveMesh = mesh;
		}

		// 2. 应用变换矩阵
		instanceUniforms.Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);

		// 3. 索引绘制
		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	void Renderer::SubmitFullscreenQuad()
	{
		// 绘制预构建的全屏四边形，通常在延迟渲染光照阶段调用
		RenderCommand::DrawTriangles(s_Data.m_FullscreenQuadVertexArray, 6);
	}

	/**
	 * @brief 指令执行中枢
	 * 负责将渲染请求派发到具体的渲染命令（当前为直接执行，未来可支持命令排序与多线程提交）。
	 */
	void Renderer::Submit(const std::function<void()>& renderFunc)
	{
		renderFunc();
	}

}
