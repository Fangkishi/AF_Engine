#include "afpch.h"
#include "Material.h"

namespace AF {

	Material::Material() 
	{

	}

	Material::~Material()
	{

	}

	//void Material::Bind()
	//{
	//	int nextTextureUnit = 0;

	//	for (const auto& [name, value] : m_Uniforms) {
	//		// Need a Visitor to apply variant values to shader's corresponding Uniforms
	//		std::visit(UniformApplier{ m_Shader, name, nextTextureUnit }, value);
	//	}
	//	// Also apply some render states like depth test, blending mode, etc.
	//	// glDisable(GL_DEPTH_TEST)...
	//}

}