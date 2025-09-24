#include "afpch.h"
#include "Mesh.h"

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

}