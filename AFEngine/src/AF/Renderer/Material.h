#pragma once

#include "AF/Renderer/Shader.h"

namespace AF {

	class Material
	{
	public:
		Material();
		~Material();

		// Set uniform value
		template<typename T>
		void SetUniform(const std::string& name, const T& value) {
			m_Uniforms[name] = value;
		}

		// Get uniform value (requires knowing the exact type)
		template<typename T>
		const T& GetUniform(const std::string& name) const {
			return std::get<T>(m_Uniforms.at(name));
		}

		bool HasUniform(const std::string& name) const {
			return m_Uniforms.find(name) != m_Uniforms.end();
		}

		const auto& GetUniforms() const { return m_Uniforms; }

		//void Bind();

    private:
        std::unordered_map<std::string, UniformValue> m_Uniforms;

        friend class Renderer;
        friend class AssimpLoader;
	};

}