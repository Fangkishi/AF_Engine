#include "afpch.h"
#include "AF/Renderer/API/VertexArray.h"

#include "AF/Renderer/RendererBackend.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace AF {
	Ref<VertexArray> VertexArray::Create()
	{
		switch (RendererBackend::GetAPI())
		{
		case RendererAPI::API::None: AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexArray>();
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
