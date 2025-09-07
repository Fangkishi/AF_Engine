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
		Ref<Texture> m_DiffuseMap;
		Ref<Texture> m_SpecularMap;

		Ref<Shader> m_DefaultShader;
		Ref<Texture> m_DefaultMap;
	};

}