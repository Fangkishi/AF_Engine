#pragma once

#include "AF/Renderer/Framebuffer.h"

#include "AF/Renderer/Texture.h"

namespace AF {

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
	};

	class RenderPass
	{
	public:
		RenderPass(const RenderPassSpecification& spec);
		~RenderPass() = default;

		const RenderPassSpecification& GetSpecification() const { return m_Specification; }

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);

	private:
		RenderPassSpecification m_Specification;
	};

}