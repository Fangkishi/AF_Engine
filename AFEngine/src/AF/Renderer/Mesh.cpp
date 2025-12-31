#include "afpch.h"
#include "Mesh.h"

//#ifndef PI
//#define PI 3.14159265359f
//#endif

namespace AF {

	Mesh::Mesh()
	{

	}

	Mesh::Mesh(const Ref<VertexArray> VertexArray, const Ref<IndexBuffer> IndexBuffer)
		: m_VertexArray(VertexArray), m_IndexBuffer(IndexBuffer), m_IndexCount(IndexBuffer->GetCount())
	{

	}

	Mesh::~Mesh()
	{

	}

	Ref<Mesh> Mesh::CreateBox(float size)
	{
		auto mesh = CreateRef<Mesh>();
		mesh->m_VertexArray = VertexArray::Create();

        float halfSize = size / 2.0f;
        // 顶点数据 (位置+法线+切线+副切线+UV坐标)
        float vertices[] = {
            // 前面 (Z+) - 法线: (0,0,1)
            -halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
             halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
             halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
            -halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

            // 后面 (Z-) - 法线: (0,0,-1)
            -halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            -halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
             halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
             halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,

             // 左面 (X-) - 法线: (-1,0,0)
             -halfSize, -halfSize, -halfSize,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
             -halfSize, -halfSize,  halfSize,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
             -halfSize,  halfSize,  halfSize,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
             -halfSize,  halfSize, -halfSize,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

             // 右面 (X+) - 法线: (1,0,0)
              halfSize, -halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
              halfSize,  halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
              halfSize,  halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
              halfSize, -halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

              // 上面 (Y+) - 法线: (0,1,0)
              -halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
              -halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
               halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
               halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

               // 下面 (Y-) - 法线: (0,-1,0)
               -halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
                halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
                halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
               -halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f
        };

        // 索引数据
        uint32_t indices[] = {
             0,  1,  2,   2,  3,  0,   // 前
             4,  5,  6,   6,  7,  4,   // 后
             8,  9, 10,  10, 11,  8,   // 左
            12, 13, 14,  14, 15, 12,   // 右
            16, 17, 18,  18, 19, 16,   // 上
            20, 21, 22,  22, 23, 20    // 下
        };

        // 创建顶点缓冲区
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },  // 位置 (location = 0)
            { ShaderDataType::Float3, "a_Normal" },    // 法线 (location = 1)
            { ShaderDataType::Float3, "a_Tangent" },   // 切线 (location = 2)
            { ShaderDataType::Float3, "a_Bitangent" }, // 副切线 (location = 3)
            { ShaderDataType::Float2, "a_TexCoord" },  // UV坐标 (location = 4)
            });

		// 创建索引缓冲区
		mesh->m_IndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		mesh->m_IndexCount = sizeof(indices) / sizeof(uint32_t);

		// 将缓冲区添加到顶点数组
		mesh->m_VertexArray->AddVertexBuffer(vertexBuffer);
		mesh->m_VertexArray->SetIndexBuffer(mesh->m_IndexBuffer);

		return mesh;
	}

    Ref<Mesh> Mesh::CreateSphere(float radius, uint32_t sectorCount, uint32_t stackCount)
    {
        auto VertexArray = VertexArray::Create();

        // 声明主要变量
        std::vector<float> positions{};
        std::vector<float> uvs{};
        std::vector<float> normals{};
        std::vector<float> tangents{};
        std::vector<float> bitangents{}; // 新增副切线容器
        std::vector<uint32_t> indices{};

        // 使用传入的参数作为经纬线数量，保留默认值
        int numLatLines = stackCount > 0 ? stackCount : 60;  // 纬线(堆叠)
        int numLongLines = sectorCount > 0 ? sectorCount : 60; // 经线(扇区)

        // 定义PI常量
        const float PI = 3.14159265358979323846f;

        // 通过两层循环生成位置、UV和法线
        for (int i = 0; i <= numLatLines; i++) {
            for (int j = 0; j <= numLongLines; j++) {
                // 计算球面坐标角度 (北极点在Y轴正方向)
                float phi = PI * 0.5f - (PI * i / numLatLines);  // 纬度角度(-π/2到π/2)
                float theta = 2 * PI * j / numLongLines;          // 经度角度(0到2π)

                // 球面坐标转笛卡尔坐标 (Y轴向上)
                float x = radius * cosf(phi) * cosf(theta);
                float y = radius * sinf(phi);                     // Y轴为高度
                float z = radius * cosf(phi) * sinf(theta);

                // UV坐标
                float u = 1.0f - (float)j / (float)numLongLines;
                float v = 1.0f - (float)i / (float)numLatLines;

                // 位置
                positions.push_back(x);
                positions.push_back(y);
                positions.push_back(z);

                // UV
                uvs.push_back(u);
                uvs.push_back(v);

                // 法线 (单位化)
                float invRadius = 1.0f / radius;
                normals.push_back(x * invRadius);
                normals.push_back(y * invRadius);
                normals.push_back(z * invRadius);
            }
        }

        // 生成索引
        for (int i = 0; i < numLatLines; i++) {
            for (int j = 0; j < numLongLines; j++) {
                int p1 = i * (numLongLines + 1) + j;
                int p2 = p1 + (numLongLines + 1);
                int p3 = p1 + 1;
                int p4 = p2 + 1;

                // 避免极点附近的三角形退化
                if (i > 0) {
                    indices.push_back(p1);
                    indices.push_back(p2);
                    indices.push_back(p3);
                }

                if (i < numLatLines - 1) {
                    indices.push_back(p3);
                    indices.push_back(p2);
                    indices.push_back(p4);
                }
            }
        }

        // 切线计算
        tangents.resize(positions.size());
        for (int i = 0; i < indices.size(); i += 3) {
            // 取出当前三角形的三个顶点的索引
            int idx0 = indices[i];
            int idx1 = indices[i + 1];
            int idx2 = indices[i + 2];

            // 获取三个顶点的位置信息
            glm::vec3 p0(positions[idx0 * 3], positions[idx0 * 3 + 1], positions[idx0 * 3 + 2]);
            glm::vec3 p1(positions[idx1 * 3], positions[idx1 * 3 + 1], positions[idx1 * 3 + 2]);
            glm::vec3 p2(positions[idx2 * 3], positions[idx2 * 3 + 1], positions[idx2 * 3 + 2]);

            // 获取三个顶点的uv信息
            glm::vec2 uv0(uvs[idx0 * 2], uvs[idx0 * 2 + 1]);
            glm::vec2 uv1(uvs[idx1 * 2], uvs[idx1 * 2 + 1]);
            glm::vec2 uv2(uvs[idx2 * 2], uvs[idx2 * 2 + 1]);

            // 计算边向量
            glm::vec3 e0 = p1 - p0;
            glm::vec3 e1 = p2 - p0;

            // 计算UV差值
            glm::vec2 dUV0 = uv1 - uv0;
            glm::vec2 dUV1 = uv2 - uv0;

            // 计算切线
            float f = 1.0f / (dUV0.x * dUV1.y - dUV1.x * dUV0.y);
            glm::vec3 tangent;
            tangent.x = f * (dUV1.y * e0.x - dUV0.y * e1.x);
            tangent.y = f * (dUV1.y * e0.y - dUV0.y * e1.y);
            tangent.z = f * (dUV1.y * e0.z - dUV0.y * e1.z);
            tangent = glm::normalize(tangent);

            // 获取法线
            glm::vec3 n0(normals[idx0 * 3], normals[idx0 * 3 + 1], normals[idx0 * 3 + 2]);
            glm::vec3 n1(normals[idx1 * 3], normals[idx1 * 3 + 1], normals[idx1 * 3 + 2]);
            glm::vec3 n2(normals[idx2 * 3], normals[idx2 * 3 + 1], normals[idx2 * 3 + 2]);

            // 正交化切线
            tangent = glm::normalize(tangent - glm::dot(tangent, n0) * n0);

            // 累加到每个顶点的切线
            tangents[idx0 * 3] += tangent.x;
            tangents[idx0 * 3 + 1] += tangent.y;
            tangents[idx0 * 3 + 2] += tangent.z;

            tangents[idx1 * 3] += tangent.x;
            tangents[idx1 * 3 + 1] += tangent.y;
            tangents[idx1 * 3 + 2] += tangent.z;

            tangents[idx2 * 3] += tangent.x;
            tangents[idx2 * 3 + 1] += tangent.y;
            tangents[idx2 * 3 + 2] += tangent.z;
        }

        // 归一化切线并并计算副切线
        bitangents.resize(tangents.size());
        for (int i = 0; i < tangents.size(); i += 3) {
            // 归一化切线
            glm::vec3 tangent(tangents[i], tangents[i + 1], tangents[i + 2]);
            tangent = glm::normalize(tangent);
            tangents[i] = tangent.x;
            tangents[i + 1] = tangent.y;
            tangents[i + 2] = tangent.z;

            // 计算副切线 (法线与切线的叉乘)
            glm::vec3 normal(normals[i], normals[i + 1], normals[i + 2]);
            glm::vec3 bitangent = glm::cross(normal, tangent);
            bitangents[i] = bitangent.x;
            bitangents[i + 1] = bitangent.y;
            bitangents[i + 2] = bitangent.z;
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

        // 创建顶点缓冲区
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },  // 位置
            { ShaderDataType::Float3, "a_Normal" },    // 法线
            { ShaderDataType::Float3, "a_Tangent" },   // 切线
            { ShaderDataType::Float3, "a_Bitangent" }, // 副切线
            { ShaderDataType::Float2, "a_TexCoord" }   // UV坐标
            });

        // 创建索引缓冲区
        auto IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());

        // 将缓冲区添加到顶点数组
        VertexArray->AddVertexBuffer(vertexBuffer);
        VertexArray->SetIndexBuffer(IndexBuffer);

        auto mesh = CreateRef<Mesh>(VertexArray, IndexBuffer);

        return mesh;
    }

}