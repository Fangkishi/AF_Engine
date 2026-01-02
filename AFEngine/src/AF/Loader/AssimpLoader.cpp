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
        const aiScene* ai_scene = importer.ReadFile(path, 
            aiProcess_Triangulate |           // 将所有图元三角化 (确保仅使用三角形面)
            aiProcess_GenSmoothNormals |      // 生成平滑法线 (如果模型没有法线，则生成它们)
            aiProcess_CalcTangentSpace |      // 计算切线和副切线空间 (用于法线贴图计算)
            aiProcess_JoinIdenticalVertices | // 合并相同顶点 (去除冗余数据)
            aiProcess_ImproveCacheLocality |  // 优化缓存局部性 (提升 GPU 渲染性能)
            aiProcess_OptimizeMeshes          // 优化网格 (合并小网格以减少 Draw Calls)
        );
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
        std::vector<float> normals;
        std::vector<float> tangents;
        std::vector<float> bitangents;
        std::vector<float> uvs;
        std::vector<uint32_t> indices;

        for (int i = 0; i < aiMesh->mNumVertices; i++) {
            //第i个顶点的位置
            positions.push_back(aiMesh->mVertices[i].x);
            positions.push_back(aiMesh->mVertices[i].y);
            positions.push_back(aiMesh->mVertices[i].z);

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

            if (aiMesh->mBitangents) {
                bitangents.push_back(aiMesh->mBitangents[i].x);
                bitangents.push_back(aiMesh->mBitangents[i].y);
                bitangents.push_back(aiMesh->mBitangents[i].z);
            }
            else {
                bitangents.push_back(0.0f);
                bitangents.push_back(0.0f);
                bitangents.push_back(0.0f);
            }

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

        // 合并所有顶点属性数据 (位置+法线+切线+副切线+UV)
        std::vector<float> vertices;
        size_t vertexCount = positions.size() / 3;
        for (size_t i = 0; i < vertexCount; i++) {
            // 位置 (3个分量)
            vertices.push_back(positions[i * 3]);
            vertices.push_back(positions[i * 3 + 1]);
            vertices.push_back(positions[i * 3 + 2]);

            // 法线 (3个分量)
            vertices.push_back(normals[i * 3]);
            vertices.push_back(normals[i * 3 + 1]);
            vertices.push_back(normals[i * 3 + 2]);

            // 切线 (3个分量)
            vertices.push_back(tangents[i * 3]);
            vertices.push_back(tangents[i * 3 + 1]);
            vertices.push_back(tangents[i * 3 + 2]);

            // 副切线 (3个分量)
            vertices.push_back(bitangents[i * 3]);
            vertices.push_back(bitangents[i * 3 + 1]);
            vertices.push_back(bitangents[i * 3 + 2]);

            // UV (2个分量)
            vertices.push_back(uvs[i * 2]);
            vertices.push_back(uvs[i * 2 + 1]);
        }

        // 创建几何数据
        Ref<VertexArray> VertexArray = VertexArray::Create();

        // 创建顶点缓冲区
        Ref<VertexBuffer> VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));
        VertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },  // 位置 (location = 0)
            { ShaderDataType::Float3, "a_Normal" },    // 法线 (location = 1)
            { ShaderDataType::Float3, "a_Tangent" },   // 切线 (location = 2)
            { ShaderDataType::Float3, "a_Bitangent" }, // 副切线 (location = 3)
            { ShaderDataType::Float2, "a_TexCoord" },  // UV坐标 (location = 4)
            });

        // 索引缓冲区
        Ref<IndexBuffer> IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());

        VertexArray->AddVertexBuffer(VertexBuffer);
        VertexArray->SetIndexBuffer(IndexBuffer);

        // 创建网格
        meshComponent.mesh = CreateRef<Mesh>(VertexArray, IndexBuffer);

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

        // 加载基础颜色/反照率贴图
        aiString aiAlbedoPath;
        if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiAlbedoPath) == AI_SUCCESS) {
                Ref<Texture2D> albedoTexture = ProcessTexture(aiMaterial, aiTextureType_DIFFUSE, aiScene, directory);
                if (albedoTexture) {
                    materialComponent.material->SetUniform("u_AlbedoMap", albedoTexture);
                    materialComponent.material->SetUniform("u_Material.UseAlbedoMap", 1);
                }
                else {
                    materialComponent.material->SetUniform("u_Material.UseAlbedoMap", 0);
                }
            }
        }
        else {
            materialComponent.material->SetUniform("u_Material.UseAlbedoMap", 0);
        }

        // 加载法线贴图
        aiString aiNormalPath;
        if (aiMaterial->GetTextureCount(aiTextureType_NORMALS) > 0 ||
            aiMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0) {

            aiTextureType normalType = aiMaterial->GetTextureCount(aiTextureType_NORMALS) > 0
                ? aiTextureType_NORMALS
                : aiTextureType_HEIGHT;

            if (aiMaterial->GetTexture(normalType, 0, &aiNormalPath) == AI_SUCCESS) {
                Ref<Texture2D> normalTexture = ProcessTexture(aiMaterial, normalType, aiScene, directory);
                if (normalTexture) {
                    materialComponent.material->SetUniform("u_NormalMap", normalTexture);
                    materialComponent.material->SetUniform("u_Material.UseNormalMap", 1);
                }
                else {
                    materialComponent.material->SetUniform("u_Material.UseNormalMap", 0);
                }
            }
        }
        else {
            materialComponent.material->SetUniform("u_Material.UseNormalMap", 0);
        }

        // 加载金属度贴图
        aiString aiMetallicPath;
        bool hasMetallicMap = false;
        if (aiMaterial->GetTextureCount(aiTextureType_METALNESS) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_METALNESS, 0, &aiMetallicPath) == AI_SUCCESS) {
                Ref<Texture2D> metallicTexture = ProcessTexture(aiMaterial, aiTextureType_METALNESS, aiScene, directory);
                if (metallicTexture) {
                    materialComponent.material->SetUniform("u_MetallicMap", metallicTexture);
                    materialComponent.material->SetUniform("u_Material.UseMetallicMap", 1);
                    hasMetallicMap = true;
                }
            }
        }

        // 加载粗糙度贴图
        aiString aiRoughnessPath;
        bool hasRoughnessMap = false;
        if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &aiRoughnessPath) == AI_SUCCESS) {
                Ref<Texture2D> roughnessTexture = ProcessTexture(aiMaterial, aiTextureType_DIFFUSE_ROUGHNESS, aiScene, directory);
                if (roughnessTexture) {
                    materialComponent.material->SetUniform("u_RoughnessMap", roughnessTexture);
                    materialComponent.material->SetUniform("u_Material.UseRoughnessMap", 1);
                    hasRoughnessMap = true;
                }
            }
        }

        // 加载环境光遮蔽贴图
        aiString aiAOPath;
        bool hasAOMap = false;
        if (aiMaterial->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0) {
            if (aiMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aiAOPath) == AI_SUCCESS) {
                Ref<Texture2D> aoTexture = ProcessTexture(aiMaterial, aiTextureType_AMBIENT_OCCLUSION, aiScene, directory);
                if (aoTexture) {
                    materialComponent.material->SetUniform("u_AOMap", aoTexture);
                    materialComponent.material->SetUniform("u_Material.UseAOMap", 1);
                    hasAOMap = true;
                }
            }
        }

        // 尝试加载ARM贴图（环境光遮蔽、粗糙度、金属度组合贴图）
        if (!hasMetallicMap || !hasRoughnessMap || !hasAOMap) {
            // 检查是否有组合贴图
            if (aiMaterial->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
                // 有些格式使用未知类型存储ARM贴图
                aiString armPath;
                if (aiMaterial->GetTexture(aiTextureType_UNKNOWN, 0, &armPath) == AI_SUCCESS) {
                    Ref<Texture2D> armTexture = ProcessTexture(aiMaterial, aiTextureType_UNKNOWN, aiScene, directory);
                    if (armTexture) {
                        materialComponent.material->SetUniform("u_ARMMap", armTexture);
                    }
                }
            }
        }

        // 设置材质颜色属性
        aiColor3D albedoColor(0.8f, 0.8f, 0.8f); // 默认灰色
        if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, albedoColor) == AI_SUCCESS) {
            materialComponent.material->SetUniform("u_Material.AlbedoColor", glm::vec4(
                albedoColor.r,
                albedoColor.g,
                albedoColor.b,
                1.0f
            ));
        }
        else {
            materialComponent.material->SetUniform("u_Material.AlbedoColor", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        }

        // 设置PBR材质参数
        float metallic = 0.0f;
        if (aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) != AI_SUCCESS) {
            // 如果没有金属度因子，尝试从镜面反射颜色推断
            aiColor3D specularColor(1.0f, 1.0f, 1.0f);
            if (aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
                // 简单的启发式方法：镜面反射颜色的亮度可以作为金属度的参考
                metallic = (specularColor.r + specularColor.g + specularColor.b) / 3.0f;
            }
        }
        materialComponent.material->SetUniform("u_Material.Metallic", metallic);

        float roughness = 0.5f;
        if (aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != AI_SUCCESS) {
            // 如果没有粗糙度因子，从光泽度转换
            float shininess = 32.0f;
            if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
                // 将光泽度转换为粗糙度（反向关系）
                roughness = 1.0f - (shininess / 100.0f);
            }
        }
        materialComponent.material->SetUniform("u_Material.Roughness", roughness);

        // 环境光遮蔽默认值
        materialComponent.material->SetUniform("u_Material.AmbientOcclusion", 1.0f);

        // 设置其他材质标识
        if (!hasMetallicMap) materialComponent.material->SetUniform("u_Material.UseMetallicMap", 0);
        if (!hasRoughnessMap) materialComponent.material->SetUniform("u_Material.UseRoughnessMap", 0);
        if (!hasAOMap) materialComponent.material->SetUniform("u_Material.UseAOMap", 0);
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