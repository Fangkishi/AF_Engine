#include "afpch.h"
#include "Material.h"

namespace AF {

	Material::Material() 
		: m_DefaultShader(Shader::Create("assets/shaders/phong.glsl")), m_DefaultMap(Texture2D::Create("assets/textures/defaultTexture.jpg"))
	{

	}

	Material::~Material()
	{

	}

	void Material::Bind()
	{
		if (!m_Shader)
		{
			m_Shader = m_DefaultShader;
		}
		m_Shader->Bind();

		// °ó¶¨ÎÆÀí
		if (!m_DiffuseMap)
		{
			m_DiffuseMap = m_DefaultMap;
		}
		m_DiffuseMap->Bind(0);
		m_Shader->SetInt("u_DiffuseMap", 0);

		//if (!m_SpecularMap)
		//{
		//	m_SpecularMap = m_DefaultMap;
		//}
		//m_SpecularMap->Bind(0);
		//m_Shader->SetInt("u_SpecularMap", 0);
	}

}