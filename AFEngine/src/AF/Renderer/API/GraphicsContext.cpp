#include "afpch.h"
#include "AF/Renderer/API/GraphicsContext.h"

#include "AF/Renderer/RendererBackend.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace AF {
	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (RendererBackend::GetAPI())
		{
		case RendererAPI::API::None: AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL: return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
