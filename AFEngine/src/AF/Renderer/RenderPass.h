#pragma once
#include "AF/Renderer/Framebuffer.h"
#include "AF/Renderer/UniformContainer.h"

namespace AF {

	/**
	 * @struct RenderPassSpecification
	 * @brief 渲染通道配置，包含目标帧缓冲和所使用的着色器
	 */
	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
		Ref<Shader> m_Shader;
	};

	/**
	 * @class RenderPass
	 * @brief 渲染通道类，封装了一次绘制过程所需的资源（Framebuffer, Shader）和全局参数（Uniforms）。
	 */
	class RenderPass : public UniformContainer
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