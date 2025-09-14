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
			glm::vec3 ambient = glm::vec3(0.2f);
			glm::vec3 diffuse = glm::vec3(0.5f);
			glm::vec3 specular = glm::vec3(1.0f);
		};
		DirLight DirLightBuffer;
	};

	static RendererData s_Data;

	void Renderer::Init()
	{
		AF_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();

		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 1);
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
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		AF_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewPosition = camera.GetPosition();
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::SubmitQuad(const Ref<Material>& material, const glm::mat4& transform)
	{
		bool depthTest = true;
		if (material)
		{
			material->Bind();

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
			material->Bind();
		}

		s_Data.m_FullscreenQuadVertexArray->Bind();
		//Renderer::DrawIndexed(6, depthTest
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const glm::mat4& transform, int entityID)
	{
		auto material = overridematerial ? overridematerial : mesh->GetMaterial();
		// 绑定材质的uniforms和纹理
		material->Bind();
		auto shader = material->GetShader();
		shader->SetMat4("u_Transform", transform);
		shader->SetInt("u_EntityID", entityID);

		shader->SetFloat3("u_DirLight.direction", s_Data.DirLightBuffer.direction);
		shader->SetFloat3("u_DirLight.ambient", s_Data.DirLightBuffer.ambient);
		shader->SetFloat3("u_DirLight.diffuse", s_Data.DirLightBuffer.diffuse);
		shader->SetFloat3("u_DirLight.specular", s_Data.DirLightBuffer.specular);
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
