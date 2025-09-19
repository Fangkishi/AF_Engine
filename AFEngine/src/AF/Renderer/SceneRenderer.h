#pragma once
#include "AF/Scene/Scene.h"

#include "AF/Renderer/RenderPass.h"
#include "AF/Renderer/UniformBuffer.h"

namespace AF {

	class SceneRenderer
	{
	public:
		static void Init(); 

		static void OnViewportResize(uint32_t width, uint32_t height);

		static void BeginScene(const Ref<Scene>& scene);
		static void EndScene();

		//static void SubmitEntity(Entity* entity);

		static Ref<Texture2D> GetFinalColorBuffer();

		static uint32_t GetFinalColorBufferRendererID();

		static int ReadPixel(int x, int y);

		static Ref<UniformBuffer> GetCameraUniformBuffer();
	private:
		static void FlushDrawList();
		static void GeometryPass();
		static void CompositePass();
	};

}