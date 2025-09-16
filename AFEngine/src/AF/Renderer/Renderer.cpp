#include "afpch.h"
#include "Renderer.h"
#include "AF/Renderer/Renderer2D.h"

#include <glad/glad.h>

namespace AF {

	struct RendererData
	{
		Ref<VertexArray> m_FullscreenQuadVertexArray;

		struct CameraData
		{
			glm::vec3 ViewPosition;
			unsigned int padding;
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

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

		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(s_Data.CameraBuffer), 0);

		s_Data.DirLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(s_Data.DirLightBuffer), 0);
		s_Data.PointLightUniformBuffer = ShaderStorageBuffer::Create(sizeof(s_Data.PointLightBuffer), 1);

		auto material = CreateRef<Material>();
		material->SetShader(Shader::Create("assets/shaders/phong.glsl"));
		s_Data.DefaultMaterial = material;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		AF_PROFILE_FUNCTION();

		glm::mat4 viewMatrix = glm::inverse(transform);
		s_Data.CameraBuffer.ViewPosition = glm::vec3(viewMatrix[3]);
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(s_Data.CameraBuffer));
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		AF_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewPosition = camera.GetPosition();
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(s_Data.CameraBuffer));
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::SubmitQuad(const Ref<Material>& material, const glm::mat4& transform)
	{
		bool depthTest = true;
		if (material)
		{
			//material->Bind();

			auto shader = material->GetShader();
			shader->SetMat4("u_Transform", transform);
		}

		s_Data.m_FullscreenQuadVertexArray->Bind();
		//Renderer::DrawIndexed(6, depthTest);
	}

	void Renderer::SubmitFullscreenQuad(const Ref<Material>& material)
	{
		bool depthTest = true;
		if (material)
		{
			//material->Bind();
		}

		s_Data.m_FullscreenQuadVertexArray->Bind();
		//Renderer::DrawIndexed(6, depthTest
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const glm::mat4& transform, int entityID)
	{
		auto material = overridematerial ? overridematerial : s_Data.DefaultMaterial;

		// 绑定材质的uniforms和纹理
		if (!material->GetShader())
			material->SetShader(s_Data.DefaultMaterial->GetShader());
		material->Bind();

		auto shader = material->GetShader();
		material->SetUniform("u_Transform", transform);
		auto normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));
		material->SetUniform("u_NormalMatrix", normalMatrix);
		material->SetUniform("u_EntityID", entityID);

		material->SetUniform("Camera", s_Data.CameraUniformBuffer);

		s_Data.DirLightUniformBuffer->SetData(&s_Data.DirLightBuffer, sizeof(s_Data.DirLightBuffer));
		material->SetUniform("DirLights", s_Data.DirLightUniformBuffer);

		s_Data.PointLightUniformBuffer->SetData(&s_Data.PointLightBuffer, sizeof(s_Data.PointLightBuffer));
		material->SetUniform("PointLights", s_Data.PointLightUniformBuffer);

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
