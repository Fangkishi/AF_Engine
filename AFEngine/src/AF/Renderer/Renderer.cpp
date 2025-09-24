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

		int texUnit = 0;
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
		DefaultMaterial->SetUniform("u_Material.UseAlbedoMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseNormalMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseMetallicMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseRoughnessMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseAOMap", 0);
		// 加载纹理
		auto albedoTexture = Texture2D::Create("assets/textures/red_brick_diff_4k.jpg");
		auto normalTexture = Texture2D::Create("assets/textures/red_brick_nor_gl_4k.jpg");
		auto armTexture = Texture2D::Create("assets/textures/red_brick_arm_4k.jpg");

		if (albedoTexture && normalTexture && armTexture) {
			DefaultMaterial->SetUniform("u_AlbedoMap", albedoTexture);
			DefaultMaterial->SetUniform("u_NormalMap", normalTexture);
			DefaultMaterial->SetUniform("u_MetallicRoughnessMap", armTexture); // ARM纹理包含金属度和粗糙度
		}
		else {
			AF_CORE_WARN("Failed to load one or more PBR textures");
		}
		s_Data.DefaultMaterial = DefaultMaterial;

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
		shader->SetMat3("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(transform))));
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
