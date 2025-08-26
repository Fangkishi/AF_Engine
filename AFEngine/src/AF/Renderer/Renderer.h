#pragma once

#include "RenderCommand.h"

#include "AF/Renderer/Camera/Camera.h"
#include "Shader.h"

namespace AF {

	class Renderer
	{
	public:
		static void Init();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(Ref<Camera> camera);
		static void EndScene();

		static void Submit(const Ref<Shader>& shader, const AF::Ref<VertexArray>& vexterArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};
		static SceneData* m_SceneData;
	};
}


