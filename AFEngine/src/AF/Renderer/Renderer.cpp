#include "afpch.h"
#include "Renderer.h"
#include "AF/Renderer/Renderer2D.h"

#include <glad/glad.h>

namespace AF {

	struct RendererData
	{
		Ref<RenderPass> m_ActiveRenderPass;

		Ref<VertexArray> m_FullscreenQuadVertexArray;

		Ref<Material> DefaultMaterial;
		Ref<Shader> DefaultShader;

		int pipelineUnit = 0;
		int materialUnit = 0;
	};

	static RendererData s_Data;

	void Renderer::Init()
	{
		AF_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
		SceneRenderer::Init();

		auto DefaultMaterial = CreateRef<Material>();
		DefaultMaterial->SetUniform("u_Material.AlbedoColor", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
		DefaultMaterial->SetUniform("u_Material.Metallic", 0.04f);
		DefaultMaterial->SetUniform("u_Material.Roughness", 0.8f);
		DefaultMaterial->SetUniform("u_Material.AmbientOcclusion", 1.0f);
		DefaultMaterial->SetUniform("u_Material.UseAlbedoMap", 1);
		DefaultMaterial->SetUniform("u_Material.UseNormalMap", 1);
		DefaultMaterial->SetUniform("u_Material.UseMetallicMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseRoughnessMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseAOMap", 0);
		// Load textures
		auto albedoTexture = Texture2D::Create("assets/textures/red_brick_diff_4k.jpg");
		auto normalTexture = Texture2D::Create("assets/textures/red_brick_nor_gl_4k.jpg");
		auto armTexture = Texture2D::Create("assets/textures/red_brick_arm_4k.jpg");

		if (albedoTexture && normalTexture && armTexture) {
			DefaultMaterial->SetUniform("u_AlbedoMap", albedoTexture);
			DefaultMaterial->SetUniform("u_NormalMap", normalTexture);
			DefaultMaterial->SetUniform("u_ARMMap", armTexture); // ARM contains ambient occlusion, roughness and metallic
		}
		else {
			AF_CORE_WARN("Failed to load one or more PBR textures");
		}
		s_Data.DefaultMaterial = DefaultMaterial;

		s_Data.DefaultShader = Shader::Create("assets/shaders/phong.glsl");

		// Create fullscreen quad vertex array
		s_Data.m_FullscreenQuadVertexArray = VertexArray::Create();

		// Create fullscreen quad vertex buffer
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

		// Apply pipeline uniforms
		ApplyUniforms(s_Data.m_ActiveRenderPass->GetSpecification().m_Shader, s_Data.m_ActiveRenderPass->GetSpecification().PassUniforms, true);
	}

	void Renderer::EndRenderPass()
	{
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();

		s_Data.pipelineUnit = 0;
		s_Data.materialUnit = 0;
	}

	void Renderer::SubmitFullscreenQuad()
	{
		RenderCommand::DrawTriangles(s_Data.m_FullscreenQuadVertexArray, 6);
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const glm::mat4& transform, int entityID)
	{
		auto& material = overridematerial ? overridematerial : s_Data.DefaultMaterial;
		auto& shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;

		// Apply material uniforms
		s_Data.materialUnit = 0;
		ApplyUniforms(shader, material->GetUniforms(), false);

		// Apply entity uniforms
		shader->SetMat4("u_Transform", transform);
		shader->SetMat3("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(transform))));
		shader->SetInt("u_EntityID", entityID);

		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	void Renderer::SubmitMeshShadow(const Ref<Mesh>& mesh, const glm::mat4& transform)
	{
		auto& shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;

		shader->SetMat4("u_Transform", transform);

		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	void Renderer::Submit(const std::function<void()>& renderFunc)
	{
		renderFunc();
	}

	void Renderer::ApplyUniforms(const Ref<Shader>& shader, const std::unordered_map<std::string, UniformValue>& m_Uniforms, const bool isPipeline)
	{
		if (!shader) return;

		for (const auto& [name, value] : m_Uniforms) {
			std::visit(UniformApplier{ shader, name, isPipeline, s_Data.pipelineUnit, s_Data.materialUnit }, value);
		}
	}

}
