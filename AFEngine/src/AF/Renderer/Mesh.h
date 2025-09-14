#pragma once
 
#include "AF/Renderer/Buffer.h"
#include "AF/Renderer/VertexArray.h"
#include "AF/Renderer/Material.h"

namespace AF {

	class Mesh
	{
	public:
		Mesh();
		Mesh(const Ref<VertexArray> VertexArray, Ref<IndexBuffer> IndexBuffer);
		~Mesh();

		static Ref<Mesh> CreateBox(float size);

		Ref<Material> GetMaterial();

	public:
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		uint32_t m_IndexCount = 0;

		Ref<Material> m_DefaultMaterial;
	};

}