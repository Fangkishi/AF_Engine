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

		auto material = CreateRef<Material>();
		material->SetShader(Shader::Create("assets/shaders/phong.glsl"));
		s_Data.DefaultMaterial = material;

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
	}

	void Renderer::EndRenderPass()
	{
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
	}

	void Renderer::SubmitFullscreenQuad(const Ref<Material>& material)
	{
		if (material)
		{
			material->Bind();
		}

		RenderCommand::DrawTriangles(s_Data.m_FullscreenQuadVertexArray, 6);
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const glm::mat4& transform, int entityID)
	{
		auto material = overridematerial ? overridematerial : s_Data.DefaultMaterial;

		// 绑定材质的uniforms和纹理
		material->SetUniform("u_Transform", transform);
		auto normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));
		material->SetUniform("u_NormalMatrix", normalMatrix);
		material->SetUniform("u_EntityID", entityID);

		material->SetUniform("Camera", SceneRenderer::GetCameraUniformBuffer());

		s_Data.DirLightUniformBuffer->SetData(&s_Data.DirLightBuffer, sizeof(s_Data.DirLightBuffer));
		material->SetUniform("DirLights", s_Data.DirLightUniformBuffer);

		s_Data.PointLightUniformBuffer->SetData(&s_Data.PointLightBuffer, sizeof(s_Data.PointLightBuffer));
		material->SetUniform("PointLights", s_Data.PointLightUniformBuffer);

		material->Bind();

		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	void Renderer::Submit(const std::function<void()>& renderFunc)
	{
		renderFunc();
	}

}
