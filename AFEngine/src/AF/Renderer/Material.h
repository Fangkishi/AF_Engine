#pragma once

#include "AF/Renderer/Shader.h"
#include "AF/Renderer/Texture.h"
#include "AF/Renderer/UniformBuffer.h"

namespace AF {

    // UniformValue 可以存储这些类型
    using UniformValue = std::variant<
        int,
        float,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::mat3,
        glm::mat4,
        Ref<Texture2D>, // 纹理也可以作为一种特殊的Uniform
        Ref<UniformBuffer>,
        Ref<ShaderStorageBuffer>
        // ... 其他需要的类型
    >;

	class Material
	{
	public:
		Material();
		~Material();

        // 设置Uniform值
        template<typename T>
        void SetUniform(const std::string& name, const T& value) {
            m_Uniforms[name] = value;
        }

        // 获取Uniform值 (使用前需知道确切类型)
        template<typename T>
        const T& GetUniform(const std::string& name) const {
            return std::get<T>(m_Uniforms.at(name));
        }

        bool HasUniform(const std::string& name) const {
            return m_Uniforms.find(name) != m_Uniforms.end();
        }

		void Bind();

        std::shared_ptr<Shader> GetShader() const { return m_Shader; }
        void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }
    private:
        std::unordered_map<std::string, UniformValue> m_Uniforms;
		Ref<Shader> m_Shader;

        // 使用Visitor模式将值应用到Shader
        struct UniformApplier {
            Ref<Shader> shader;
            const std::string& name;
            int& nextTextureUnit;

            void operator()(int value) const { shader->SetInt(name, value); }
            void operator()(float value) const { shader->SetFloat(name, value); }
            void operator()(const glm::vec2& value) const { shader->SetFloat2(name, value); }
            void operator()(const glm::vec3& value) const { shader->SetFloat3(name, value); }
            void operator()(const glm::vec4& value) const { shader->SetFloat4(name, value); }
            void operator()(const glm::mat3& value) const { shader->SetMat3(name, value); }
            void operator()(const glm::mat4& value) const { shader->SetMat4(name, value); }
            void operator()(const std::shared_ptr<Texture2D>& texture) const {
                if (texture) {
                    if (nextTextureUnit >= 32) {
                        AF_CORE_ERROR("Texture unit limit exceeded for uniform: {}", name);
                        return;
                    }
                    int texUnit = nextTextureUnit++;
                    texture->Bind(texUnit);
                    shader->SetInt(name, texUnit);
                }
                else {
                    AF_CORE_WARN("Attempting to bind null texture for uniform: {}", name);
                }
            }
            void operator()(const Ref<UniformBuffer>& ubo) const {
                ubo->Bind();
            }
            void operator()(const Ref<ShaderStorageBuffer>& ssbo) const {
                ssbo->Bind();
            }
            // ... 为variant中的每种类型重载 operator()
        };

        friend class Renderer;
        friend class AssimpLoader;
	};

}