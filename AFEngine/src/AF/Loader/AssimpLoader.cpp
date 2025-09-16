#include "afpch.h"
#include "AssimpLoader.h"
#include "AF/Math/Math.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

namespace AF {

    std::unordered_map<std::string, Ref<Texture2D>> AssimpLoader::s_TextureCache;

	void AssimpLoader::Load(const std::string& path, const Ref<Scene>& scene) {
		//拿出模型所在目录
		std::size_t lastIndex = path.find_last_of("//");
		auto rootPath = path.substr(0, lastIndex + 1);

		Assimp::Importer importer;
        const aiScene* ai_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

		//验证读取是否正确顺利
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
    ) {
        Entity entity;

        // 检查是否是根节点 (根节点的父节点为空且名称为"RootNode")
        bool isRootNode = (!parentEntity && std::string(aiNode->mName.C_Str()) == "RootNode");

        if (!isRootNode) {
            // 不是根节点才创建实体
            entity = Scene->CreateEntity(aiNode->mName.C_Str());

            // 设置变换组件
            auto& transform = entity.GetComponent<TransformComponent>();
            glm::mat4 localMatrix = GetMat4f(aiNode->mTransformation);
            glm::vec3 translation, rotation, scale;
            AF::Math::DecomposeTransform(localMatrix, transform.Translation, transform.Rotation, transform.Scale);

            // 添加父子层级关系组件
            auto& hierarchy = entity.AddComponent<ParentChildComponent>();
            if (parentEntity)
            {
                hierarchy.ParentID = parentEntity.GetUUID();
                auto& parentHierarchy = parentEntity.GetComponent<ParentChildComponent>();
                parentHierarchy.AddChild(entity.GetUUID());
            }
        }
        else {
            // 如果是根节点，则使用父节点作为当前处理的实体
            entity = parentEntity;
        }

        // 处理当前节点的网格
        for (unsigned int i = 0; i < aiNode->mNumMeshes; i++) {
            aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
            if (!isRootNode) {
                ProcessMesh(entity, aiMesh, aiScene, directory);
            }
        }

        // 递归处理所有子节点
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
        // 创建网格组件
        auto& meshComponent = entity.AddComponent<MeshComponent>();

        // 处理顶点数据
        std::vector<float> positions;
        std::vector<float> uvs;
        std::vector<float> normals;
        std::vector<float> tangents;
        std::vector<uint32_t> indices;

        for (int i = 0; i < aiMesh->mNumVertices; i++) {
            //第i个顶点的位置
            positions.push_back(aiMesh->mVertices[i].x);
            positions.push_back(aiMesh->mVertices[i].y);
            positions.push_back(aiMesh->mVertices[i].z);

            //第i个顶点的UV
            //关注第0套UV, 一般是贴图UV
            if (aiMesh->mTextureCoords[0]) {
                uvs.push_back(aiMesh->mTextureCoords[0][i].x);
                uvs.push_back(aiMesh->mTextureCoords[0][i].y);
            }
            else {
                uvs.push_back(1.0f);
                uvs.push_back(1.0f);
            }

            //第i个顶点的法线
            normals.push_back(aiMesh->mNormals[i].x);
            normals.push_back(aiMesh->mNormals[i].y);
            normals.push_back(aiMesh->mNormals[i].z);

            // 从Assimp网格中获取切线
            if (aiMesh->mTangents) {
                tangents.push_back(aiMesh->mTangents[i].x);
                tangents.push_back(aiMesh->mTangents[i].y);
                tangents.push_back(aiMesh->mTangents[i].z);
            }
            else {
                tangents.push_back(0.0f);
                tangents.push_back(0.0f);
                tangents.push_back(0.0f);
            }
        }

        // 处理索引
        for (uint32_t f = 0; f < aiMesh->mNumFaces; f++)
        {
            aiFace	face = aiMesh->mFaces[f];
            for (uint32_t id = 0; id < face.mNumIndices; id++)
            {
                indices.push_back(face.mIndices[id]);
            }
        }

        // 创建几何数据
        Ref<VertexArray> vertexArray = VertexArray::Create();

        // 创建顶点缓冲区
        bool hasNormals = !normals.empty();
        bool hasTexCoords = !uvs.empty();
        bool hasTangents = !tangents.empty() && aiMesh->mTangents;

        // 位置缓冲区
        std::vector<float> positionData(positions.begin(), positions.end());
        Ref<VertexBuffer> positionBuffer = VertexBuffer::Create(positionData.data(), positionData.size() * sizeof(float));
        BufferLayout positionLayout({
            { ShaderDataType::Float3, "a_Position" }
            });
        positionBuffer->SetLayout(positionLayout);
        vertexArray->AddVertexBuffer(positionBuffer);

        // 纹理坐标缓冲区
        if (hasTexCoords) {
            std::vector<float> texCoordData(uvs.begin(), uvs.end());
            Ref<VertexBuffer> texCoordBuffer = VertexBuffer::Create(texCoordData.data(), texCoordData.size() * sizeof(float));
            BufferLayout texCoordLayout({
                { ShaderDataType::Float2, "a_TexCoord" }
                });
            texCoordBuffer->SetLayout(texCoordLayout);
            vertexArray->AddVertexBuffer(texCoordBuffer);
        }

        // 法线缓冲区
        if (hasNormals) {
            std::vector<float> normalData(normals.begin(), normals.end());
            Ref<VertexBuffer> normalBuffer = VertexBuffer::Create(normalData.data(), normalData.size() * sizeof(float));
            BufferLayout normalLayout({
                { ShaderDataType::Float3, "a_Normal" }
                });
            normalBuffer->SetLayout(normalLayout);
            vertexArray->AddVertexBuffer(normalBuffer);
        }

        // 切线缓冲区
        if (hasTangents) {
            std::vector<float> tangentData(tangents.begin(), tangents.end());
            Ref<VertexBuffer> tangentBuffer = VertexBuffer::Create(tangentData.data(), tangentData.size() * sizeof(float));
            BufferLayout tangentLayout({
                { ShaderDataType::Float3, "a_Tangent" }
                });
            tangentBuffer->SetLayout(tangentLayout);
            vertexArray->AddVertexBuffer(tangentBuffer);
        }

        // 索引缓冲区
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
        vertexArray->SetIndexBuffer(indexBuffer);

        // 创建网格
        meshComponent.mesh = CreateRef<Mesh>(vertexArray, indexBuffer);

        // 处理材质
        if (aiMesh->mMaterialIndex >= 0)
        {
            aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
            ProcessMaterial(entity, aiMaterial, aiScene, directory);
        }
        else
        {
            auto& materialComponent = entity.AddComponent<MaterialComponent>();
        }
    }

    void AssimpLoader::ProcessMaterial(
        Entity entity,
        aiMaterial* aiMaterial,
        const aiScene* aiScene,
        const std::string& directory
    ) {
        // 创建材质组件
        auto& materialComponent = entity.AddComponent<MaterialComponent>();
        materialComponent.material = CreateRef<Material>();

        // 加载漫反射贴图
        aiString aiDiffusePath;
        if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiDiffusePath) == AI_SUCCESS) {
                Ref<Texture2D> diffuseTexture = ProcessTexture(aiMaterial, aiTextureType_DIFFUSE, aiScene, directory);
                if (diffuseTexture) {
                    materialComponent.material->SetUniform("u_DiffuseMap", diffuseTexture);
                }
                else {
                    // 加载失败使用默认贴图
                    materialComponent.material->SetUniform("u_DiffuseMap", Texture2D::Create("assets/textures/defaultTexture.jpg"));
                }
            }
        }
        else {
            // 没有漫反射贴图，使用默认贴图
            materialComponent.material->SetUniform("u_DiffuseMap", Texture2D::Create("assets/textures/defaultTexture.jpg"));
        }

        // 加载镜面贴图
        aiString aiSpecularPath;
        if (aiMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &aiSpecularPath) == AI_SUCCESS) {
                Ref<Texture2D> specularTexture = ProcessTexture(aiMaterial, aiTextureType_SPECULAR, aiScene, directory);
                if (specularTexture) {
                    materialComponent.material->SetUniform("u_SpecularMap", specularTexture);
                }
                else {
                    // 加载失败使用默认贴图
                    materialComponent.material->SetUniform("u_SpecularMap", Texture2D::Create("assets/textures/defaultTexture.jpg"));
                }
            }
        }
        else {
            // 没有镜面贴图，使用默认贴图
            materialComponent.material->SetUniform("u_SpecularMap", Texture2D::Create("assets/textures/defaultTexture.jpg"));
        }

        // 获取材质颜色属性
        aiColor3D diffuseColor(0.8f, 0.8f, 0.8f); // 默认灰色
        if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
            //materialComponent.material->m_DiffuseColor = glm::vec3(
            //    diffuseColor.r,
            //    diffuseColor.g,
            //    diffuseColor.b
            //);
        }

        aiColor3D specularColor(1.0f, 1.0f, 1.0f); // 默认白色
        if (aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
            materialComponent.material->SetUniform("u_Material.specularColor", glm::vec3(
                specularColor.r,
                specularColor.g,
                specularColor.b
            ));
        }

        float shininess = 32.0f;
        if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            materialComponent.material->SetUniform("u_Material.shininess", shininess);
        }
    }

    Ref<Texture2D> AssimpLoader::ProcessTexture(
        const aiMaterial* aiMaterial,
        aiTextureType type,
        const aiScene* aiScene,
        const std::string& directory
    ) {
        // 获取图片获取路径
        aiString aipath;
        aiMaterial->Get(AI_MATKEY_TEXTURE(type, 0), aipath);
        if (!aipath.length) {
            return nullptr;
        }

        // 检查纹理是否已缓存
        std::string textureKey = std::string(aipath.C_Str());
        if (s_TextureCache.find(textureKey) != s_TextureCache.end()) {
            return s_TextureCache[textureKey];
        }

        // 判断是否是嵌入fbx的图片
        const aiTexture* aitexture = aiScene->GetEmbeddedTexture(aipath.C_Str());
        if (aitexture) {
            // 图片数据是嵌入的
            if (aitexture->mHeight == 0) {
                // 压缩格式如JPEG, PNG等
                // 使用stb_image在内存中加载和解码
                int width, height, channels;
                stbi_uc* data = stbi_load_from_memory(
                    reinterpret_cast<stbi_uc*>(aitexture->pcData),
                    aitexture->mWidth, // 这里是压缩数据的大小
                    &width, &height, &channels, 0);

                if (!data) {
                    AF_CORE_ERROR("Failed to decode embedded texture: {}", aipath.C_Str());
                    return nullptr;
                }

                // 确定格式
                ImageFormat format = ImageFormat::None;
                if (channels == 3) {
                    format = ImageFormat::RGB8;
                }
                else if (channels == 4) {
                    format = ImageFormat::RGBA8;
                }
                else {
                    AF_CORE_ERROR("Unsupported number of channels: {}", channels);
                    stbi_image_free(data);
                    return nullptr;
                }

                // 创建纹理规范
                TextureSpecification spec;
                spec.Width = width;
                spec.Height = height;
                spec.Format = format;
                spec.GenerateMips = true;

                // 创建纹理
                Ref<Texture2D> texture = Texture2D::Create(spec);

                // 计算数据大小
                uint32_t dataSize = width * height * channels;

                // 上传纹理数据
                texture->SetData((void*)data, dataSize);

                stbi_image_free(data);

                // 缓存纹理
                s_TextureCache[textureKey] = texture;
                return texture;
            }
            else {
                // 未压缩格式
                unsigned char* dataIn = reinterpret_cast<unsigned char*>(aitexture->pcData);
                uint32_t widthIn = aitexture->mWidth;
                uint32_t heightIn = aitexture->mHeight;

                // 根据achFormatHint确定格式
                ImageFormat format = ImageFormat::RGBA8; // 默认RGBA8

                // 根据格式提示
                std::string formatHint(aitexture->achFormatHint);
                if (formatHint.find("rgb") != std::string::npos) {
                    format = ImageFormat::RGB8;
                }
                else if (formatHint.find("rgba") != std::string::npos) {
                    format = ImageFormat::RGBA8;
                }

                // 创建纹理规范
                TextureSpecification spec;
                spec.Width = widthIn;
                spec.Height = heightIn;
                spec.Format = format;
                spec.GenerateMips = true;

                // 创建纹理
                Ref<Texture2D> texture = Texture2D::Create(spec);

                // 计算数据大小
                uint32_t bpp = (format == ImageFormat::RGBA8) ? 4 : 3;
                uint32_t dataSize = widthIn * heightIn * bpp;

                // 上传纹理数据
                texture->SetData((void*)dataIn, dataSize);

                // 缓存纹理
                s_TextureCache[textureKey] = texture;
                return texture;
            }
        }
        else {
            // 外部图片文件
            std::string fullPath = directory + aipath.C_Str();

            // 检查文件是否存在
            if (s_TextureCache.find(fullPath) != s_TextureCache.end()) {
                return s_TextureCache[fullPath];
            }

            Ref<Texture2D> texture = Texture2D::Create(fullPath);
            if (texture) {
                s_TextureCache[fullPath] = texture;
            }
            return texture;
        }

        return nullptr;
    }

	glm::mat4 AssimpLoader::GetMat4f(aiMatrix4x4 value) {
		glm::mat4 to(
			value.a1, value.a2, value.a3, value.a4,
			value.b1, value.b2, value.b3, value.b4,
			value.c1, value.c2, value.c3, value.c4,
			value.d1, value.d2, value.d3, value.d4
		);

		return to;
	}

}