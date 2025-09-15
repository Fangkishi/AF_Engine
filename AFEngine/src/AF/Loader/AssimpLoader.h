#pragma once
#include "AF/Scene/Entity.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace AF {

	class AssimpLoader
	{
	public:
		static void Load(const std::string& Path, const Ref<Scene>& Scene);

	private:
		static std::unordered_map<std::string, Ref<Texture2D>> s_TextureCache;

		static void ProcessNode(
			Ref<Scene> Scene,
			aiNode* aiNode,
			Entity parentEntity,
			const aiScene* aiScene,
			const std::string& directory
		);

		static void ProcessMesh(
			Entity entity,
			aiMesh* aiMesh,
			const aiScene* aiScene,
			const std::string& directory
		);

		static void ProcessMaterial(
			Entity entity,
			aiMaterial* aiMaterial,
			const aiScene* aiScene,
			const std::string& directory
		);

		static Ref<Texture2D> ProcessTexture(
			const aiMaterial* aiMaterial,
			aiTextureType type,
			const aiScene* aiScene,
			const std::string& directory
		);

		static glm::mat4 getMat4f(aiMatrix4x4 value);
	};

}