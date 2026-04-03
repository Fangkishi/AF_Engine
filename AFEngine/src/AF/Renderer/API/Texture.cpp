#include "afpch.h"
#include "AF/Renderer/API/Texture.h"

#include "AF/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace AF {
	Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(specification);
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, const bool isSRGB)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(path, isSRGB);
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(uint32_t rendererID, uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(rendererID, width, height);
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const TextureSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTextureCube>(specification);
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const std::string& path, const bool isSRGB)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTextureCube>(path, isSRGB);
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(uint32_t rendererID, uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AF_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTextureCube>(rendererID, width, height);
		}

		AF_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	// TextureLibrary -------------------------------------------------------------------------
	static std::map<std::string, Ref<Texture2D>> s_TextureCache;

	void TextureLibrary::LoadTexture(const std::string& path, bool isSRGB)
	{
		std::string key = path + (isSRGB ? "_srgb" : "_linear");
		if (s_TextureCache.find(key) == s_TextureCache.end())
		{
			Ref<Texture2D> texture = Texture2D::Create(path, isSRGB);
			if (texture->IsLoaded())
			{
				s_TextureCache[key] = texture;
			}
		}
	}

	Ref<Texture2D> TextureLibrary::GetTexture(const std::string& path, bool isSRGB)
	{
		std::string key = path + (isSRGB ? "_srgb" : "_linear");
		if (s_TextureCache.find(key) != s_TextureCache.end())
		{
			return s_TextureCache[key];
		}

		Ref<Texture2D> texture = Texture2D::Create(path, isSRGB);
		if (texture->IsLoaded())
		{
			s_TextureCache[key] = texture;
			return texture;
		}

		return nullptr;
	}

	bool TextureLibrary::Exists(const std::string& path, bool isSRGB)
	{
		std::string key = path + (isSRGB ? "_srgb" : "_linear");
		return s_TextureCache.find(key) != s_TextureCache.end();
	}

}
