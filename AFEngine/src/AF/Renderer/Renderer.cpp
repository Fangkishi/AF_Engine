#include "afpch.h"
#include "Renderer.h"
#include "AF/Renderer/Renderer2D.h"

#include <glad/glad.h>

namespace AF {

	struct RendererData
	{
		Ref<RenderPass> m_ActiveRenderPass;

		Ref<VertexArray> m_FullscreenQuadVertexArray;

		struct DirLight
		{
			glm::vec3 direction = glm::vec3(-1.0f, -1.0f, -1.0f);
			float pad1;
			glm::vec3 ambient = glm::vec3(0.2f);
			float pad2;
			glm::vec3 diffuse = glm::vec3(1.5f);
			float pad3;
			glm::vec3 specular = glm::vec3(1.0f);
			float pad4;
		};
		DirLight DirLightBuffer;
		Ref<ShaderStorageBuffer> DirLightUniformBuffer;

		struct PointLight
		{
			glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f);
			float padding1 = 0.0f; 
			glm::vec3 color = glm::vec3(100.0f, 100.0f, 100.0f);
			float intensity = 10.0f;
		};
		PointLight PointLightBuffer[10];
		Ref<ShaderStorageBuffer> PointLightUniformBuffer;

		Ref<Material> DefaultMaterial;
		Ref<Shader> DefaultShader;

		int texUnit = 0;
	};

	static RendererData s_Data;

	void Renderer::Init()
	{
		AF_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
		SceneRenderer::Init();

		s_Data.DirLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(s_Data.DirLightBuffer), 0);
		s_Data.PointLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(s_Data.PointLightBuffer), 1);

		s_Data.DefaultMaterial = CreateRef<Material>();;
		s_Data.DefaultShader = Shader::Create("assets/shaders/phong.glsl");

		// 创建全屏四边形顶点数组
		s_Data.m_FullscreenQuadVertexArray = VertexArray::Create();

		// 定义全屏四边形顶点数据
		float vertices[] = {
			// positions        // texCoords
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,

			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
		};

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		vertexBuffer->SetLayout(layout);
		s_Data.m_FullscreenQuadVertexArray->AddVertexBuffer(vertexBuffer);
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene()
	{

	}

	void Renderer::EndScene()
	{

	}

	void Renderer::BeginRenderPass(const Ref<RenderPass> renderPass)
	{
		s_Data.m_ActiveRenderPass = renderPass;

		renderPass->GetSpecification().TargetFramebuffer->Bind();

		Ref<Shader> shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;
		if (!shader) {
			shader = s_Data.DefaultShader;
		}

		shader->Bind();

		// 全局参数
		//TODO:全局参数整合成一个方法
		SceneRenderer::GetCameraUniformBuffer()->Bind();

		s_Data.DirLightUniformBuffer->SetData(&s_Data.DirLightBuffer, sizeof(s_Data.DirLightBuffer));
		s_Data.DirLightUniformBuffer->Bind();

		s_Data.PointLightUniformBuffer->SetData(&s_Data.PointLightBuffer, sizeof(s_Data.PointLightBuffer));
		s_Data.PointLightUniformBuffer->Bind();

		// 通道参数
		ApplyUniforms(s_Data.m_ActiveRenderPass->GetSpecification().m_Shader, s_Data.m_ActiveRenderPass->GetSpecification().PassUniforms);
	}

	void Renderer::EndRenderPass()
	{
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();

		s_Data.texUnit = 0;
	}

	void Renderer::SubmitFullscreenQuad(const Ref<Material>& material)
	{
		if (material)
		{
			//material->Bind();
		}

		RenderCommand::DrawTriangles(s_Data.m_FullscreenQuadVertexArray, 6);
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const glm::mat4& transform, int entityID)
	{
		auto& material = overridematerial ? overridematerial : s_Data.DefaultMaterial;
		auto& shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;

		// 材质参数
		ApplyUniforms(shader, material->GetUniforms());

		// 实体参数
		shader->SetMat4("u_Transform", transform);
		auto normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));
		shader->SetMat3("u_NormalMatrix", normalMatrix);
		shader->SetInt("u_EntityID", entityID);

		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	void Renderer::Submit(const std::function<void()>& renderFunc)
	{
		renderFunc();
	}

	void Renderer::ApplyUniforms(const Ref<Shader>& shader, const std::unordered_map<std::string, UniformValue>& m_Uniforms)
	{
		if (!shader) return;

		for (const auto& [name, value] : m_Uniforms) {
			std::visit(UniformApplier{ shader, name, s_Data.texUnit }, value);
		}
	}

}
