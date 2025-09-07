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
		// �������� (λ��+����+��������)
		float vertices[] = {
			// ǰ��
			-halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
			 halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
			 halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
			-halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

			// ����
			-halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			-halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
			 halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			 halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,

			 // ����
			 -halfSize, -halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
			 -halfSize, -halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
			 -halfSize,  halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			 -halfSize,  halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

			 // ����
			  halfSize, -halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
			  halfSize,  halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			  halfSize,  halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			  halfSize, -halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

			  // ����
			  -halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
			  -halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
			   halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
			   halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,

			   // ����
			   -halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
				halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
				halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
			   -halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
		};

		// ��������
		uint32_t indices[] = {
			 0,  1,  2,   2,  3,  0,   // ǰ
			 4,  5,  6,   6,  7,  4,   // ��
			 8,  9, 10,  10, 11,  8,   // ��
			12, 13, 14,  14, 15, 12,   // ��
			16, 17, 18,  18, 19, 16,   // ��
			20, 21, 22,  22, 23, 20    // ��
		};

		// �������㻺����
		mesh->m_VertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		mesh->m_VertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			});

		// ��������������
		mesh->m_IndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		mesh->m_IndexCount = sizeof(indices) / sizeof(uint32_t);

		// ����������ӵ���������
		mesh->m_VertexArray->AddVertexBuffer(mesh->m_VertexBuffer);
		mesh->m_VertexArray->SetIndexBuffer(mesh->m_IndexBuffer);

		return mesh;
	}

	Ref<Material> Mesh::GetMaterial() {
		return m_DefaultMaterial;
	}

}