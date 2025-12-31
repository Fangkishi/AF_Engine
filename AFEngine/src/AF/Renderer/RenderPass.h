#pragma once

#include "AF/Renderer/Framebuffer.h"
#include "AF/Renderer/Shader.h"

namespace AF {

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
		Ref<Shader> m_Shader;
		std::unordered_map<std::string, UniformValue> PassUniforms;
	};

	class RenderPass
	{
	public:
		RenderPass(const RenderPassSpecification& spec);
		~RenderPass() = default;

		// 设置Uniform值
		template<typename T>
		void SetUniform(const std::string& name, const T& value) {
			m_Specification.PassUniforms[name] = value;
		}

		// 获取Uniform值 (使用前需知道确切类型)
		template<typename T>
		const T& GetUniform(const std::string& name) const {
			return std::get<T>(m_Specification.PassUniforms.at(name));
		}

		bool HasUniform(const std::string& name) const {
			return m_Specification.PassUniforms.find(name) != m_Specification.PassUniforms.end();
		}

		const RenderPassSpecification& GetSpecification() const { return m_Specification; }

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);

	private:
		RenderPassSpecification m_Specification;
	};

}