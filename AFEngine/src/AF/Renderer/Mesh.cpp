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
        // �������� (λ��+UV+����+����)
        float vertices[] = {
            // ǰ�� (Z������) - ����: (1,0,0)
            -halfSize, -halfSize,  halfSize,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,
             halfSize, -halfSize,  halfSize,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,
             halfSize,  halfSize,  halfSize,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,
            -halfSize,  halfSize,  halfSize,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,

            // ���� (Z������) - ����: (-1,0,0) ����U��ת
            -halfSize, -halfSize, -halfSize,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,
            -halfSize,  halfSize, -halfSize,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,
             halfSize,  halfSize, -halfSize,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,
             halfSize, -halfSize, -halfSize,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,  -1.0f, 0.0f, 0.0f,

             // ���� (X������) - ����: (0,0,-1)
             -halfSize, -halfSize, -halfSize,  0.0f, 0.0f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,
             -halfSize, -halfSize,  halfSize,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,
             -halfSize,  halfSize,  halfSize,  1.0f, 1.0f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,
             -halfSize,  halfSize, -halfSize,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, -1.0f,

             // ���� (X������) - ����: (0,0,1)
              halfSize, -halfSize, -halfSize,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
              halfSize,  halfSize, -halfSize,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
              halfSize,  halfSize,  halfSize,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
              halfSize, -halfSize,  halfSize,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,

              // ���� (Y������) - ����: (1,0,0) ����V��ת
              -halfSize,  halfSize, -halfSize,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,
              -halfSize,  halfSize,  halfSize,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,
               halfSize,  halfSize,  halfSize,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,
               halfSize,  halfSize, -halfSize,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,

               // ���� (Y������) - ����: (1,0,0)
               -halfSize, -halfSize, -halfSize,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 0.0f,
                halfSize, -halfSize, -halfSize,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 0.0f,
                halfSize, -halfSize,  halfSize,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 0.0f,
               -halfSize, -halfSize,  halfSize,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 0.0f
        };

        // �������ݱ��ֲ���
        uint32_t indices[] = {
             0,  1,  2,   2,  3,  0,   // ǰ
             4,  5,  6,   6,  7,  4,   // ��
             8,  9, 10,  10, 11,  8,   // ��
            12, 13, 14,  14, 15, 12,   // ��
            16, 17, 18,  18, 19, 16,   // ��
            20, 21, 22,  22, 23, 20    // ��
        };

        // �������㻺����
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },  // λ��
            { ShaderDataType::Float2, "a_TexCoord" },  // UV����
            { ShaderDataType::Float3, "a_Normal" },    // ����
            { ShaderDataType::Float3, "a_Tangent" },   // ����
            });

		// ��������������
		mesh->m_IndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		mesh->m_IndexCount = sizeof(indices) / sizeof(uint32_t);

		// ����������ӵ���������
		mesh->m_VertexArray->AddVertexBuffer(vertexBuffer);
		mesh->m_VertexArray->SetIndexBuffer(mesh->m_IndexBuffer);

		return mesh;
	}

}