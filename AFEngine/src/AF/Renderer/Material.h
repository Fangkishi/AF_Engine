#pragma once

#include "AF/Renderer/Shader.h"
#include "AF/Renderer/Texture.h"

namespace AF {

	class Material
	{
	public:
		Material();
		~Material();

		void Bind();

		Ref<Shader> GetShader() { return m_Shader; }
	public:
		Ref<Shader> m_Shader;
        Ref<Texture2D> m_DiffuseMap;
        Ref<Texture2D> m_SpecularMap;
        Ref<Texture2D> m_NormalMap;
        Ref<Texture2D> m_EmissiveMap;

        glm::vec3 m_DiffuseColor = glm::vec3(1.0f);
        glm::vec3 m_SpecularColor = glm::vec3(1.0f);
        glm::vec3 m_EmissiveColor = glm::vec3(0.0f);

        float m_Shininess = 32.0f;
        float m_Metallic = 0.0f;
        float m_Roughness = 0.5f;
        float m_Opacity = 1.0f;

        // 材质属性标识
        bool m_HasDiffuseMap = false;
        bool m_HasSpecularMap = false;
        bool m_HasNormalMap = false;
        bool m_HasEmissiveMap = false;

		Ref<Shader> m_DefaultShader;
		Ref<Texture2D> m_DefaultMap;
	};

}