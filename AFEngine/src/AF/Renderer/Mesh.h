#pragma once
 
#include "AF/Renderer/Buffer.h"
#include "AF/Renderer/VertexArray.h"
#include "AF/Renderer/Material.h"
#include "AF/Renderer/UniformContainer.h"

namespace AF {

	/**
	 * @class Mesh
	 * @brief 网格类，包含顶点数据、索引数据以及网格级的 Uniform 变量。
	 */
	class Mesh : public UniformContainer
	{
	public:
		Mesh();
		Mesh(const Ref<VertexArray> VertexArray, Ref<IndexBuffer> IndexBuffer);
		~Mesh();

		static Ref<Mesh> CreateBox(const float size);
		static Ref<Mesh> CreateSphere(const float radius, const uint32_t sectorCount = 100, const uint32_t stackCount = 50);

	public:
		Ref<VertexArray> m_VertexArray;
		Ref<IndexBuffer> m_IndexBuffer;
		uint32_t m_IndexCount = 0;

		friend class Renderer;
		friend class AssimpLoader;
	};

}