#include "afpch.h"
#include "GraphicsContext.h"

#include "AF/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace AF {
	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL: return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
