#include "afpch.h"
#include "Mesh.h"

namespace AF {

	Mesh::Mesh()
	{
		m_DefaultMaterial = CreateRef<Material>();
	}

	Mesh::Mesh(const Ref<VertexArray> VertexArray, const Ref<VertexBuffer> VertexBuffer,const Ref<IndexBuffer> IndexBuffer)
		: m_VertexArray(VertexArray), m_VertexBuffer(VertexBuffer), m_IndexBuffer(IndexBuffer), m_IndexCount(IndexBuffer->GetCount())
	{
		m_DefaultMaterial = CreateRef<Material>();
	}

	Mesh::~Mesh()
	{

	}

	Ref<Mesh> Mesh::CreateBox(float size)
	{
		auto mesh = CreateRef<Mesh>();
		mesh->m_VertexArray = VertexArray::Create();

		float halfSize = size / 2.0f;
		// 顶点数据 (位置+法线+纹理坐标)
		float vertices[] = {
			// 前面
			-halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
			 halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
			 halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
			-halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

			// 后面
			-halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			-halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
			 halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			 halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,

			 // 左面
			 -halfSize, -halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
			 -halfSize, -halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
			 -halfSize,  halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			 -halfSize,  halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

			 // 右面
			  halfSize, -halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
			  halfSize,  halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			  halfSize,  halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			  halfSize, -halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

			  // 顶面
			  -halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
			  -halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
			   halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
			   halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,

			   // 底面
			   -halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
				halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
				halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
			   -halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
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
		mesh->m_VertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		mesh->m_VertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			});

		// 创建索引缓冲区
		mesh->m_IndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		mesh->m_IndexCount = sizeof(indices) / sizeof(uint32_t);

		// 将缓冲区添加到顶点数组
		mesh->m_VertexArray->AddVertexBuffer(mesh->m_VertexBuffer);
		mesh->m_VertexArray->SetIndexBuffer(mesh->m_IndexBuffer);

		return mesh;
	}

	Ref<Material> Mesh::GetMaterial() {
		return m_DefaultMaterial;
	}

}