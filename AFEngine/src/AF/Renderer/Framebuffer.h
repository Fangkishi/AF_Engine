#pragma once

#include "AF/Core/Base.h"
#include "AF/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace AF {

	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RGBA16F,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;

		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format)
		{
		}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		// TODO: filtering/wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;

		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments)
		{
		}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification Attachments;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		virtual uint32_t GetColorAttachmentCount() const = 0;
		virtual Ref<Texture> GetColorAttachment(uint32_t index = 0) const = 0;
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual bool HasDepthAttachment() const = 0;
		virtual Ref<Texture> GetDepthAttachment() const = 0;

		virtual void BindTexture(uint32_t slot = 0, uint32_t index = 0) const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual void AttachTextureLayer(Ref<Texture2D> texture, uint32_t attachment, uint32_t layer = 0) = 0;
		virtual void AttachCubeMapLayer(Ref<TextureCube> texture, uint32_t attachment, uint32_t face, uint32_t layer = 0) = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};

}
