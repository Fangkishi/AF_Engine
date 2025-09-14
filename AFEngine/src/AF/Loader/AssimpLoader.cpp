#include "afpch.h"
#include "AssimpLoader.h"
#include "AF/Math/Math.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace AF {

	void AssimpLoader::Load(const std::string& path, const Ref<Scene>& scene) {
		//�ó�ģ������Ŀ¼
		std::size_t lastIndex = path.find_last_of("//");
		auto rootPath = path.substr(0, lastIndex + 1);

		Assimp::Importer importer;
        const aiScene* ai_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

		//��֤��ȡ�Ƿ���ȷ˳��
		if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode) {
			std::cerr << "Error:Model Read Failed!" << std::endl;
		}

        ProcessNode(scene, ai_scene->mRootNode, Entity{}, ai_scene, rootPath);
	}

	void AssimpLoader::ProcessNode(
		Ref<Scene> Scene,
		aiNode* aiNode,
		Entity parentEntity,
		const aiScene* aiScene,
		const std::string& directory
	){
		// ������ǰ�ڵ�ʵ��
		Entity entity = Scene->CreateEntity(aiNode->mName.C_Str());

		// ���ñ任���
		auto& transform = entity.GetComponent<TransformComponent>();
		glm::mat4 localMatrix = getMat4f(aiNode->mTransformation);
		glm::vec3 translation, rotation, scale;
		AF::Math::DecomposeTransform(localMatrix, transform.Translation, transform.Rotation, transform.Scale);

		// ��Ӳ㼶���
		auto& hierarchy = entity.AddComponent<ParentChildComponent>();
        if (parentEntity)
        {
		    hierarchy.ParentID = parentEntity.GetUUID();
            auto& parentHierarchy = parentEntity.GetComponent<ParentChildComponent>();
            parentHierarchy.AddChild(entity.GetUUID());
        }

		// ����ǰ�ڵ����������
		for (unsigned int i = 0; i < aiNode->mNumMeshes; i++) {
			aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
			ProcessMesh(entity, aiMesh, aiScene, directory);
		}

		// �ݹ鴦�������ӽڵ�
		for (unsigned int i = 0; i < aiNode->mNumChildren; i++) {
			ProcessNode(Scene, aiNode->mChildren[i], entity, aiScene, directory);
		}
	}

    void AssimpLoader::ProcessMesh(
        Entity entity,
        aiMesh* aiMesh,
        const aiScene* aiScene,
        const std::string& directory
    ) {
        // �����������
        auto& meshComponent = entity.AddComponent<MeshComponent>();

        // ����������
        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<float> uvs;
        std::vector<uint32_t> indices;

        for (int i = 0; i < aiMesh->mNumVertices; i++) {
            //��i�������λ��
            positions.push_back(aiMesh->mVertices[i].x);
            positions.push_back(aiMesh->mVertices[i].y);
            positions.push_back(aiMesh->mVertices[i].z);

            //��i������ķ���
            normals.push_back(aiMesh->mNormals[i].x);
            normals.push_back(aiMesh->mNormals[i].y);
            normals.push_back(aiMesh->mNormals[i].z);

            //��i�������UV
            //��ע��0��UV, һ������ͼUV
            if (aiMesh->mTextureCoords[0]) {
                uvs.push_back(aiMesh->mTextureCoords[0][i].x);
                uvs.push_back(aiMesh->mTextureCoords[0][i].y);
            }
            else {
                uvs.push_back(1.0f);
                uvs.push_back(1.0f);
            }
        }

        // ��������
        for (uint32_t f = 0; f < aiMesh->mNumFaces; f++)
        {
            aiFace	face = aiMesh->mFaces[f];
            for (uint32_t id = 0; id < face.mNumIndices; id++)
            {
                indices.push_back(face.mIndices[id]);
            }
        }

        // ������������
        Ref<VertexArray> vertexArray = VertexArray::Create();

        // �������㻺����
        bool hasNormals = !normals.empty();
        bool hasTexCoords = !uvs.empty();
        size_t vertexCount = positions.size() / 3;
        size_t vertexSize = 3 + (hasNormals ? 3 : 0) + (hasTexCoords ? 2 : 0);

        std::vector<float> vertexData(vertexCount * vertexSize);

        for (size_t i = 0; i < vertexCount; ++i) {
            size_t offset = i * vertexSize;

            // λ��
            vertexData[offset++] = positions[i * 3];
            vertexData[offset++] = positions[i * 3 + 1];
            vertexData[offset++] = positions[i * 3 + 2];

            // ����
            if (hasNormals) {
                vertexData[offset++] = normals[i * 3];
                vertexData[offset++] = normals[i * 3 + 1];
                vertexData[offset++] = normals[i * 3 + 2];
            }

            // ��������
            if (hasTexCoords) {
                vertexData[offset++] = uvs[i * 2];
                vertexData[offset++] = uvs[i * 2 + 1];
            }
        }

        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertexData.data(), vertexData.size() * sizeof(float));
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices.data(), indices.size());

        // ���ö��㲼��
        std::vector<BufferElement> elements;
        elements.push_back({ ShaderDataType::Float3, "a_Position" });

        if (!normals.empty()) {
            elements.push_back({ ShaderDataType::Float3, "a_Normal" });
        }

        if (!uvs.empty()) {
            elements.push_back({ ShaderDataType::Float2, "a_TexCoord" });
        }

        BufferLayout layout({
            elements[0],
            (!normals.empty() ? elements[1] : BufferElement{}),
            (!uvs.empty() ? elements[2] : BufferElement{})
            });

        vertexBuffer->SetLayout(layout);
        vertexArray->AddVertexBuffer(vertexBuffer);
        vertexArray->SetIndexBuffer(indexBuffer);

        // ��������
        meshComponent.mesh = CreateRef<Mesh>(vertexArray, indexBuffer);

        // �������
        if (aiMesh->mMaterialIndex >= 0)
        {
            aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
            ProcessMaterial(entity, aiMaterial, aiScene, directory);
        }
        else
        {
            auto& materialComponent = entity.AddComponent<MaterialComponent>();
            materialComponent.material = CreateRef<Material>();
            materialComponent.material->m_DiffuseMap = Texture2D::Create("assets/textures/defaultTexture.jpg");
        }
    }

    void AssimpLoader::ProcessMaterial(
        Entity entity,
        aiMaterial* aiMaterial,
        const aiScene* aiScene,
        const std::string& directory
    ) {
        // �����������
        auto& materialComponent = entity.AddComponent<MaterialComponent>();
        materialComponent.material = CreateRef<Material>();

        // ������������ͼ
        aiString aiDiffusePath;
        if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiDiffusePath) == AI_SUCCESS) {
                Ref<Texture2D> diffuseTexture = ProcessTexture(aiMaterial, aiTextureType_DIFFUSE, aiScene, directory);
                if (diffuseTexture) {
                    materialComponent.material->m_DiffuseMap = diffuseTexture;
                }
                else {
                    // ����ʧ��ʹ��Ĭ����ͼ
                    materialComponent.material->m_DiffuseMap = Texture2D::Create("assets/textures/defaultTexture.jpg");
                }
            }
        }
        else {
            // û����������ͼ��ʹ��Ĭ����ͼ
            materialComponent.material->m_DiffuseMap = Texture2D::Create("assets/textures/defaultTexture.jpg");
        }

        // ���ؾ�����ͼ
        aiString aiSpecularPath;
        if (aiMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &aiSpecularPath) == AI_SUCCESS) {
                Ref<Texture2D> specularTexture = ProcessTexture(aiMaterial, aiTextureType_SPECULAR, aiScene, directory);
                if (specularTexture) {
                    materialComponent.material->m_SpecularMap = specularTexture;
                }
                else {
                    // ����ʧ��ʹ��Ĭ����ͼ
                    materialComponent.material->m_SpecularMap = Texture2D::Create("assets/textures/defaultTexture.jpg");
                }
            }
        }
        else {
            // û�о�����ͼ��ʹ��Ĭ����ͼ
            materialComponent.material->m_SpecularMap = Texture2D::Create("assets/textures/defaultTexture.jpg");
        }

        // ��ȡ������ɫ����
        aiColor3D diffuseColor(0.8f, 0.8f, 0.8f); // Ĭ�ϻ�ɫ
        if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
            materialComponent.material->m_DiffuseColor = glm::vec3(
                diffuseColor.r,
                diffuseColor.g,
                diffuseColor.b
            );
        }

        aiColor3D specularColor(1.0f, 1.0f, 1.0f); // Ĭ�ϰ�ɫ
        if (aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
            materialComponent.material->m_SpecularColor = glm::vec3(
                specularColor.r,
                specularColor.g,
                specularColor.b
            );
        }

        float shininess = 32.0f;
        if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            materialComponent.material->m_Shininess = shininess;
        }
    }

    Ref<Texture2D> AssimpLoader::ProcessTexture(
        const aiMaterial* aiMaterial,
        aiTextureType type,
        const aiScene* aiScene,
        const std::string& directory
    ) {
        aiString aiPath;
        if (aiMaterial->GetTexture(type, 0, &aiPath) != AI_SUCCESS) {
            return nullptr;
        }

        // ����Ƿ���Ƕ�������
        const aiTexture* embeddedTexture = aiScene->GetEmbeddedTexture(aiPath.C_Str());
        if (embeddedTexture) {
            // ������Ƕ����
            return LoadEmbeddedTexture(embeddedTexture, aiPath.C_Str());
        }
        else {
            // �����ⲿ�ļ�����
            std::string fullPath = directory + "/" + aiPath.C_Str();

            // ����ļ��Ƿ����
            if (!std::filesystem::exists(fullPath)) {
                std::string alternativePath = fullPath;
                if (std::filesystem::exists(alternativePath)) {
                    fullPath = alternativePath;
                }
                else {
                    return nullptr; // �ļ�������
                }
            }

            return LoadExternalTexture(fullPath);
        }
    }

    Ref<Texture2D> AssimpLoader::LoadEmbeddedTexture(const aiTexture* embeddedTexture, const std::string& name) {
        try {
            // ��������ʽ
            if (embeddedTexture->mHeight == 0) {
                // ѹ����ʽ����JPEG, PNG��
                //return Texture2D::CreateFromMemory(
                //    name,
                //    reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
                //    embeddedTexture->mWidth
                //);
                return nullptr;
            }
            else {
                // δѹ����ARGB��ʽ
                //std::vector<unsigned char> imageData;
                //imageData.reserve(embeddedTexture->mWidth * embeddedTexture->mHeight * 4);

                //for (uint32_t y = 0; y < embeddedTexture->mHeight; y++) {
                //    for (uint32_t x = 0; x < embeddedTexture->mWidth; x++) {
                //        aiTexel texel = embeddedTexture->pcData[y * embeddedTexture->mWidth + x];
                //        imageData.push_back(texel.r);
                //        imageData.push_back(texel.g);
                //        imageData.push_back(texel.b);
                //        imageData.push_back(texel.a);
                //    }
                //}

                //return Texture2D::CreateFromData(
                //    name,
                //    imageData.data(),
                //    embeddedTexture->mWidth,
                //    embeddedTexture->mHeight,
                //    4
                //);
                return nullptr;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to load embedded texture: " << e.what() << std::endl;
            return nullptr;
        }
    }

    Ref<Texture2D> AssimpLoader::LoadExternalTexture(const std::string& filePath) {
        try {
            return Texture2D::Create(filePath);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to load external texture: " << filePath << " - " << e.what() << std::endl;
            return nullptr;
        }
    }

	glm::mat4 AssimpLoader::getMat4f(aiMatrix4x4 value) {
		glm::mat4 to(
			value.a1, value.a2, value.a3, value.a4,
			value.b1, value.b2, value.b3, value.b4,
			value.c1, value.c2, value.c3, value.c4,
			value.d1, value.d2, value.d3, value.d4
		);

		return to;
	}

}