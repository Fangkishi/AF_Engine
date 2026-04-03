#pragma once

#include "AF/Renderer/API/Framebuffer.h"

namespace AF {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint32_t GetColorAttachmentCount() const override;
		virtual Ref<Texture> GetColorAttachment(uint32_t index = 0) const override;
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override
		{
			AF_CORE_ASSERT(index < m_ColorAttachments.size(), "");
			return m_ColorAttachments[index];
		}

		virtual bool HasDepthAttachment() const override;
		virtual Ref<Texture> GetDepthAttachment() const override;

		virtual void BindTexture(uint32_t slot = 0, uint32_t index = 0) const override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		virtual void AttachTexture(Ref<Texture> texture, uint32_t attachment = 0) override;
		virtual void AttachTextureLayer(Ref<Texture2D> texture, uint32_t attachment, uint32_t layer = 0) override;
		virtual void AttachCubeMapLayer(Ref<TextureCube> texture, uint32_t attachment, uint32_t face, uint32_t layer = 0) override;

	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;

		std::unordered_map<uint32_t, Ref<Texture>> m_ExternalColorTextures; // 颜色附着索引 -> 外部纹理
		Ref<Texture> m_ExternalDepthTexture; // 外部深度纹理
	};

}
