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

	public:
		Ref<VertexArray> m_VertexArray;
		Ref<IndexBuffer> m_IndexBuffer;
		uint32_t m_IndexCount = 0;

		friend class Renderer;
		friend class AssimpLoader;
	};

}