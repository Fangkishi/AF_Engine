#include "afpch.h"
#include "SceneRenderer.h"
#include "AF/Renderer/Renderer.h"
#include "AF/Renderer/Renderer2D.h"

namespace AF {

	struct SceneRendererData
	{
		Ref<Scene> ActiveScene;

		Ref<RenderPass> GeoPass;
		Ref<RenderPass> CompositePass;

		float Exposure = 1.0f;
		struct CameraData
		{
			glm::vec3 ViewPosition;
			unsigned int padding;
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		Ref<Shader> CompositeShader;
	};

	static SceneRendererData s_Data;

	void SceneRenderer::Init()
	{
		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Width = 1280;
		geoFramebufferSpec.Height = 720;
		geoFramebufferSpec.Attachments = {
			FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth
		};

		// Geometry Render Pass Specification
		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.TargetFramebuffer = Framebuffer::Create(geoFramebufferSpec);
		s_Data.GeoPass = RenderPass::Create(geoRenderPassSpec);

		FramebufferSpecification comFramebufferSpec;
		comFramebufferSpec.Width = 1280;
		comFramebufferSpec.Height = 720;
		comFramebufferSpec.Attachments = {
			FramebufferTextureFormat::RGBA8
		};

		// Geometry Render Pass Specification
		RenderPassSpecification comRenderPassSpec;
		comRenderPassSpec.TargetFramebuffer = Framebuffer::Create(comFramebufferSpec);
		s_Data.CompositePass = RenderPass::Create(comRenderPassSpec);

		s_Data.CompositeShader = Shader::Create("assets/shaders/hdr.glsl");

		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(s_Data.CameraBuffer), 0);
	}

	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
	}

	void SceneRenderer::BeginScene(const Ref<Scene>& scene)
	{
		s_Data.ActiveScene = scene;

		glm::mat4 viewMatrix = s_Data.ActiveScene->GetCamera()->GetViewMatrix();
		s_Data.CameraBuffer.ViewPosition = glm::vec3(viewMatrix[3]);
		s_Data.CameraBuffer.ViewProjection = s_Data.ActiveScene->GetCamera()->GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(s_Data.CameraBuffer));
	}

	void SceneRenderer::EndScene()
	{
		FlushDrawList();

		s_Data.ActiveScene = nullptr;
	}

	//void SceneRenderer::SubmitEntity(Entity* entity)
	//{
	//	// 提交实体以供渲染
	//}

	Ref<Texture2D> SceneRenderer::GetFinalColorBuffer()
	{
		// 返回最终的颜色缓冲区
		return s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachment(0);
		return nullptr;
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID(0);
	}

	int SceneRenderer::ReadPixel(int x, int y)
	{
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->Bind();
		// 从几何通道的第二个颜色附件（实体ID）读取像素
		int pixelData =  s_Data.GeoPass->GetSpecification().TargetFramebuffer->ReadPixel(1, x, y);
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->Unbind();
		return pixelData;
	}

	Ref<UniformBuffer> SceneRenderer::GetCameraUniformBuffer()
	{
		return s_Data.CameraUniformBuffer;
	}

	void SceneRenderer::FlushDrawList()
	{
		// 执行几何通道和合成通道
		GeometryPass();
		CompositePass();
	}

	void SceneRenderer::GeometryPass()
	{
		// 几何通道
		Renderer::BeginRenderPass(s_Data.GeoPass);

		RenderCommand::Clear();

		// Clear our entity ID attachment to -1
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->ClearAttachment(1, -1);

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

		Renderer::EndRenderPass();
	}

	void SceneRenderer::CompositePass()
	{
		// 合成通道
		Renderer::BeginRenderPass(s_Data.CompositePass);

		RenderCommand::Clear();

		s_Data.CompositeShader->Bind();
		s_Data.CompositeShader->SetFloat("u_Exposure ", s_Data.Exposure);
		s_Data.CompositeShader->SetInt("u_SceneTexture ", 0);

		// 获取几何通道的结果
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
		Renderer::SubmitFullscreenQuad(nullptr);

		Renderer::EndRenderPass();
	}

}