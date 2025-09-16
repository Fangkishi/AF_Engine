#include "afpch.h"
#include "Material.h"

namespace AF {

	Material::Material() 
	{

	}

	Material::~Material()
	{

	}

	void Material::Bind()
	{
		m_Shader->Bind();

		int nextTextureUnit = 0;

		for (const auto& [name, value] : m_Uniforms) {
			// ��Ҫһ��Visitor����variantֵӦ�õ�shader�Ķ�ӦUniform��
			std::visit(UniformApplier{ m_Shader, name, nextTextureUnit }, value);
		}
		// ͬʱ��������һЩ����״̬������Ȳ��ԡ����ģʽ��
		// glDisable(GL_DEPTH_TEST)...
	}

}