#include "afpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace AF {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}