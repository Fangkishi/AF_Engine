#include "afpch.h"
#include "AF/Renderer/UniformContainer.h"
#include "AF/Renderer/Texture.h"
#include "AF/Renderer/UniformBuffer.h"
#include <variant>

namespace AF {

	/**
	 * @struct UniformApplier
	 * @brief 内部访问者实现，用于将 UniformValue 应用到 Shader
	 */
	struct UniformApplier {
		Ref<Shader> shader;
		const std::string& name;
		bool isPipeline;
		int& pipelineUnit;
		int& materialUnit;
		TextureUnitCache& cache;

		void operator()(int value) const { shader->SetInt(name, value); }
		void operator()(float value) const { shader->SetFloat(name, value); }
		void operator()(const glm::vec2& value) const { shader->SetFloat2(name, value); }
		void operator()(const glm::vec3& value) const { shader->SetFloat3(name, value); }
		void operator()(const glm::vec4& value) const { shader->SetFloat4(name, value); }
		void operator()(const glm::mat3& value) const { shader->SetMat3(name, value); }
		void operator()(const glm::mat4& value) const { shader->SetMat4(name, value); }

		void operator()(const Ref<Texture>& texture) const {
			if (!texture) return;

			uint32_t texID = texture->GetRendererID();
			int texUnit = -1;

			// 1. 检查缓存中是否已存在该纹理
			auto it = cache.find(texID);
			if (it != cache.end()) {
				texUnit = it->second;
			}
			else {
				// 2. 分配新的纹理单元
				texUnit = pipelineUnit + materialUnit;
				if (texUnit >= 32) {
					AF_CORE_ERROR("Texture unit limit exceeded (32) for uniform: {0}", name);
					return;
				}

				texture->Bind(texUnit);

				cache[texID] = texUnit;

				// 3. 更新计数器
				if (isPipeline) pipelineUnit++;
				else materialUnit++;
			}

			// 4. 设置着色器采样器 Uniform
			shader->SetInt(name, texUnit);
		}

		void operator()(const Ref<UniformBuffer>& ubo) const { ubo->Bind(); }
		void operator()(const Ref<ShaderStorageBuffer>& ssbo) const { ssbo->Bind(); }

		// 处理指针类型（解引用并应用）
		void operator()(const int* v) const { if (v) shader->SetInt(name, *v); }
		void operator()(const float* v) const { if (v) shader->SetFloat(name, *v); }
		void operator()(const glm::vec2* v) const { if (v) shader->SetFloat2(name, *v); }
		void operator()(const glm::vec3* v) const { if (v) shader->SetFloat3(name, *v); }
		void operator()(const glm::vec4* v) const { if (v) shader->SetFloat4(name, *v); }
		void operator()(const glm::mat3* v) const { if (v) shader->SetMat3(name, *v); }
		void operator()(const glm::mat4* v) const { if (v) shader->SetMat4(name, *v); }
	};

	void UniformContainer::Apply(const Ref<Shader>& shader, bool isPipeline, int& pipelineUnit, int& materialUnit, TextureUnitCache& cache) const
	{
		if (!shader) return;

		for (const auto& [name, value] : m_Uniforms) {
			std::visit(UniformApplier{ shader, name, isPipeline, pipelineUnit, materialUnit, cache }, value);
		}
	}

}
