#include "afpch.h"
#include "AF/Renderer/Material.h"

namespace AF {

	Material::Material() 
	{

	}

	Material::~Material()
	{

	}

	Ref<Material> Material::CreatePBR()
	{
		Ref<Material> material = CreateRef<Material>();

		// Set default PBR uniforms
		material->SetUniform("u_Material.AlbedoColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		material->SetUniform("u_Material.Metallic", 0.0f);
		material->SetUniform("u_Material.Roughness", 0.5f);
		material->SetUniform("u_Material.AmbientOcclusion", 1.0f);

		// Set default map usage flags (0 = false)
		material->SetUniform("u_Material.UseAlbedoMap", 0);
		material->SetUniform("u_Material.UseNormalMap", 0);
		material->SetUniform("u_Material.UseMetallicMap", 0);
		material->SetUniform("u_Material.UseRoughnessMap", 0);
		material->SetUniform("u_Material.UseAOMap", 0);

		// Initialize texture uniforms
		material->SetUniform("u_AlbedoMap", Ref<Texture>(nullptr));
		material->SetUniform("u_NormalMap", Ref<Texture>(nullptr));
		material->SetUniform("u_MetallicMap", Ref<Texture>(nullptr));
		material->SetUniform("u_RoughnessMap", Ref<Texture>(nullptr));
		material->SetUniform("u_AOMap", Ref<Texture>(nullptr));
		material->SetUniform("u_ARMMap", Ref<Texture>(nullptr));

		return material;
	}

}
