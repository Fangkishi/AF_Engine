#pragma once

#include "AF/Renderer/UniformContainer.h"

namespace AF {

	/**
	 * @class Material
	 * @brief 材质类，包含着色器所需的参数（Uniforms）和纹理。
	 */
	class Material : public UniformContainer
	{
	public:
		Material();
		~Material();

		static Ref<Material> CreatePBR();

	private:
		friend class Renderer;
		friend class AssimpLoader;
	};

}