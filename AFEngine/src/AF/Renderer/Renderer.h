#pragma once

#include "RenderCommand.h"

#include "AF/Renderer/Camera.h"
#include "AF/Renderer/EditorCamera.h"

#include "AF/Renderer/Mesh.h"
#include "AF/Renderer/Material.h"
#include "AF/Renderer/UniformBuffer.h"
#include "Shader.h"

namespace AF {

	class Renderer
	{
	public:
		static void Init();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void SubmitQuad(const Ref<Material>& material, const glm::mat4& transform);
		static void SubmitFullscreenQuad(const Ref<Material>& material);

		static void SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const glm::mat4& transform, int entityID = -1);
		static void Submit(const std::function<void()>& renderFunc);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:

	};
}
