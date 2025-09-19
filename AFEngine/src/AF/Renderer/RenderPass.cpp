#include "afpch.h"
#include "AF/Renderer/RenderPass.h"

namespace AF {

	RenderPass::RenderPass(const RenderPassSpecification& spec)
		: m_Specification(spec)
	{
	}

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		return CreateRef<RenderPass>(spec);
	}

}