#pragma once

#include "AF/Renderer/API/Shader.h"
#include "AF/Core/Assert.h"
#include <unordered_map>
#include <string>

namespace AF {

	/** @brief 纹理单元缓存，用于追踪纹理 ID 到已分配单元的映射 */
	using TextureUnitCache = std::unordered_map<uint32_t, int>;

	/**
	 * @class UniformContainer
	 * @brief 通用的 Uniform 变量容器，提供 Uniform 的存储、查询与设置功能。
	 * 该类被 Material 和 RenderPass 等需要管理着色器参数的类继承。
	 */
	class UniformContainer
	{
	public:
		virtual ~UniformContainer() = default;

		/**
		 * @brief 将容器内的所有参数应用到指定的着色器
		 * @param shader 目标着色器
		 * @param isPipeline 是否为管线级别 (影响纹理单元计数方式)
		 * @param pipelineUnit 管线纹理单元计数引用
		 * @param materialUnit 材质纹理单元计数引用
		 * @param cache 纹理单元缓存 (用于去重)
		 */
		void Apply(const Ref<Shader>& shader, bool isPipeline, int& pipelineUnit, int& materialUnit, TextureUnitCache& cache) const;

		/**
		 * @brief 设置 Uniform 变量的值
		 * @param name 变量名称
		 * @param value 变量值（支持 int, float, vec, mat, Texture 等）
		 */
		template<typename T>
		void SetUniform(const std::string& name, const T& value) {
			m_Uniforms[name] = value;
		}

		/**
		 * @brief 获取 Uniform 变量的值
		 * @tparam T 变量的确切类型
		 * @param name 变量名称
		 * @return 变量值的常量引用
		 */
		template<typename T>
		const T& GetUniform(const std::string& name) const {
			AF_CORE_ASSERT(HasUniform(name), "Uniform 变量未找到!");
			return std::get<T>(m_Uniforms.at(name));
		}

		/**
		 * @brief 检查是否存在指定的 Uniform 变量
		 */
		bool HasUniform(const std::string& name) const {
			return m_Uniforms.find(name) != m_Uniforms.end();
		}

		/**
		 * @brief 获取内部存储的所有 Uniform 映射表
		 */
		const std::unordered_map<std::string, UniformValue>& GetUniforms() const {
			return m_Uniforms;
		}

		/**
		 * @brief 清空所有已设置的 Uniform 变量
		 */
		void ClearUniforms() {
			m_Uniforms.clear();
		}

	protected:
		std::unordered_map<std::string, UniformValue> m_Uniforms;
	};

}
