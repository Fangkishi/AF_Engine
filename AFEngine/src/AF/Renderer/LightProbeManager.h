#pragma once

#include "AF/Core/Base.h"
#include "AF/Scene/Scene.h"
#include "AF/Renderer/API/Texture.h"

#include <glm/glm.hpp>
#include <vector>

namespace AF {

	/**
	 * @brief 光照探针网格定义
	 */
	struct LightProbeGrid
	{
		glm::vec3 MinBounds = { -10.0f, 0.0f, -10.0f };
		glm::vec3 MaxBounds = { 10.0f, 10.0f, 10.0f };
		glm::uvec3 Counts = { 5, 3, 5 }; // X, Y, Z 方向上的探针数量
		
		// 动态生成时每个探针的影响半径
		float ProbeRadius = 5.0f;
	};

	/**
	 * @class LightProbeManager
	 * @brief 负责生成、管理和烘焙光照探针，计算球谐(SH)系数
	 *        与 SceneRenderer 解耦，可插拔使用。
	 */
	class LightProbeManager
	{
	public:
		/**
		 * @brief 在场景中根据网格设置自动生成光照探针实体
		 * @param scene 目标场景
		 * @param grid 网格定义参数
		 */
		static void GenerateProbes(Ref<Scene> scene, const LightProbeGrid& grid);

		/**
		 * @brief 烘焙场景中所有的光照探针
		 *        （由于目前没有独立的 Cubemap 渲染管线，此方法暂留结构）
		 * @param scene 包含光照探针的场景
		 */
		static void BakeProbes(Ref<Scene> scene);

		/**
		 * @brief 将场景中所有探针的数据打包为 GPU 友好的格式（如 3D Texture 或 SSBO 数据）
		 *        为了实现快速三线性插值，通常推荐生成 3D Texture
		 * @param scene 场景
		 * @return 返回创建好的 Texture3D（如果引擎支持），或供 SSBO 使用的浮点数组
		 */
		static std::vector<float> GetProbesDataForGPU(Ref<Scene> scene);
	};

}
