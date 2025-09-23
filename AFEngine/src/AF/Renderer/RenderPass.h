#pragma once

#include "AF/Renderer/Framebuffer.h"
#include "AF/Renderer/Shader.h"

namespace AF {

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
		Ref<Shader> m_Shader;
		std::unordered_map<std::string, UniformValue> PassUniforms;

		// 设置Uniform值
		template<typename T>
		void SetUniform(const std::string& name, const T& value) {
			PassUniforms[name] = value;
		}

		// 获取Uniform值 (使用前需知道确切类型)
		template<typename T>
		const T& GetUniform(const std::string& name) const {
			return std::get<T>(PassUniforms.at(name));
		}

		bool HasUniform(const std::string& name) const {
			return PassUniforms.find(name) != PassUniforms.end();
		}
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