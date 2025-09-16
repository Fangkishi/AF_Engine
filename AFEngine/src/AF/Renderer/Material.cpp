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
			// 需要一个Visitor来将variant值应用到shader的对应Uniform上
			std::visit(UniformApplier{ m_Shader, name, nextTextureUnit }, value);
		}
		// 同时可以设置一些材质状态，如深度测试、混合模式等
		// glDisable(GL_DEPTH_TEST)...
	}

}