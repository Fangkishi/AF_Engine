#include "afpch.h"
#include "LightProbeManager.h"
#include "AF/Scene/Components.h"
#include "AF/Scene/Entity.h"
#include <glm/gtc/constants.hpp>
#include <random>

namespace AF {

	// -----------------------------------------------------------------------------------
	// 辅助结构与函数：简单的 CPU 光线追踪器，用于烘焙环境光
	// -----------------------------------------------------------------------------------
	
	struct AABB {
		glm::vec3 Min, Max;
		glm::vec3 Albedo;
	};

	// 射线与 AABB 盒子相交检测 (Slab Method)
	bool IntersectAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const AABB& box, float& outT, glm::vec3& outNormal)
	{
		glm::vec3 invDir = 1.0f / rayDir;
		glm::vec3 t0 = (box.Min - rayOrigin) * invDir;
		glm::vec3 t1 = (box.Max - rayOrigin) * invDir;

		glm::vec3 tmin = glm::min(t0, t1);
		glm::vec3 tmax = glm::max(t0, t1);

		float tnear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
		float tfar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

		if (tnear > tfar || tfar < 0.0f) return false;

		outT = tnear < 0.0f ? tfar : tnear;

		// 计算交点法线
		glm::vec3 p = rayOrigin + rayDir * outT;
		glm::vec3 c = (box.Min + box.Max) * 0.5f;
		glm::vec3 d = (box.Max - box.Min) * 0.5f;
		glm::vec3 localP = (p - c) / d;
		
		// 找出绝对值最大的分量作为法线方向
		glm::vec3 absP = glm::abs(localP);
		if (absP.x > absP.y && absP.x > absP.z) outNormal = glm::vec3(glm::sign(localP.x), 0, 0);
		else if (absP.y > absP.z) outNormal = glm::vec3(0, glm::sign(localP.y), 0);
		else outNormal = glm::vec3(0, 0, glm::sign(localP.z));

		return true;
	}

	// 提取场景中的静态几何体作为碰撞盒 (仅做极简的 Box 支持，因为康奈尔盒子全是 Box)
	std::vector<AABB> BuildSceneAABBs(Ref<Scene> scene)
	{
		std::vector<AABB> boxes;
		auto view = scene->GetAllEntitiesWithView<TransformComponent, MeshComponent, MaterialComponent>();
		for (auto entityID : view)
		{
			auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entityID);
			
			// 假设全都是单位立方体拉伸而成的 (Mesh::CreateBox(1.0f))
			glm::vec3 halfScale = transform.Scale * 0.5f;
			AABB box;
			box.Min = transform.Translation - halfScale;
			box.Max = transform.Translation + halfScale;
			
			// 从材质中提取基础色
			// 我们需要通过 HasUniform 检查，再通过 GetUniform 获取
			glm::vec4 albedoColor = glm::vec4(1.0f); // 默认白色
			if (material.material && material.material->HasUniform("u_Material.AlbedoColor")) {
				albedoColor = material.material->GetUniform<glm::vec4>("u_Material.AlbedoColor");
			}
			box.Albedo = glm::vec3(albedoColor);
			boxes.push_back(box);
		}
		return boxes;
	}

	// 球谐函数基底计算 (3阶)
	void EvalSHBasis(const glm::vec3& dir, float* sh)
	{
		float x = dir.x, y = dir.y, z = dir.z;
		
		// L0
		sh[0] = 0.282095f; 
		
		// L1
		// 修正：确保光线追踪时的 rayDir 与 Shader 中求值时的 normal 坐标系一致
		// 由于蒙特卡洛积分是累加入射光 (incoming radiance)，
		// 入射光方向指向外部，而我们在 Shader 中是用表面法线 (指向外部) 去查询。
		// 但是，SH 投影在编码时，如果光从方向 dir 射来，它照亮的是法线为 -dir 的表面。
		// 也就是我们需要用 -dir 来进行编码，才能让 Shader 里用法线直接求出正确的光照。
		float nx = -x, ny = -y, nz = -z;
		
		sh[1] = -0.488603f * ny;
		sh[2] = 0.488603f * nz;
		sh[3] = -0.488603f * nx;
		
		// L2
		sh[4] = 1.092548f * nx * ny;
		sh[5] = -1.092548f * ny * nz;
		sh[6] = 0.315392f * (3.0f * nz * nz - 1.0f);
		sh[7] = -1.092548f * nx * nz;
		sh[8] = 0.546274f * (nx * nx - ny * ny);
	}

	void LightProbeManager::GenerateProbes(Ref<Scene> scene, const LightProbeGrid& grid)
	{
		// 1. 清除现有的探针
		auto view = scene->GetAllEntitiesWithView<LightProbeComponent>();
		for (auto entityID : view)
		{
			scene->DestroyEntity(Entity{ entityID, scene.get() });
		}

		// 2. 计算步长
		glm::vec3 step = { 0.0f, 0.0f, 0.0f };
		if (grid.Counts.x > 1) step.x = (grid.MaxBounds.x - grid.MinBounds.x) / (float)(grid.Counts.x - 1);
		if (grid.Counts.y > 1) step.y = (grid.MaxBounds.y - grid.MinBounds.y) / (float)(grid.Counts.y - 1);
		if (grid.Counts.z > 1) step.z = (grid.MaxBounds.z - grid.MinBounds.z) / (float)(grid.Counts.z - 1);

		// 3. 均匀生成网格探针实体
		int probeIndex = 0;
		for (uint32_t z = 0; z < grid.Counts.z; ++z)
		{
			for (uint32_t y = 0; y < grid.Counts.y; ++y)
			{
				for (uint32_t x = 0; x < grid.Counts.x; ++x)
				{
					glm::vec3 position = grid.MinBounds + glm::vec3(x * step.x, y * step.y, z * step.z);

					std::string name = "LightProbe_" + std::to_string(probeIndex++);
					Entity probeEntity = scene->CreateEntity(name);
					
					auto& transform = probeEntity.GetComponent<TransformComponent>();
					transform.Translation = position;

					auto& probeComp = probeEntity.AddComponent<LightProbeComponent>();
					probeComp.Radius = grid.ProbeRadius;
					probeComp.IsBaked = false;

					// TODO: 如果需要可视化，可以添加一个只有线框的 CircleRenderer 或极小的 Sphere Mesh
				}
			}
		}

		AF_CORE_INFO("LightProbeManager: Generated {0} light probes.", probeIndex);
	}

	void LightProbeManager::BakeProbes(Ref<Scene> scene)
	{
		auto probeView = scene->GetAllEntitiesWithView<TransformComponent, LightProbeComponent>();
		auto lightView = scene->GetAllEntitiesWithView<TransformComponent, PointLightComponent>();
		
		// 提取点光源信息
		struct PointLightData { glm::vec3 pos; glm::vec3 color; };
		std::vector<PointLightData> lights;
		for (auto entityID : lightView) {
			auto [transform, light] = lightView.get<TransformComponent, PointLightComponent>(entityID);
			lights.push_back({transform.Translation, light.Color * light.Intensity});
		}

		// 构建场景 AABB
		std::vector<AABB> sceneBoxes = BuildSceneAABBs(scene);

		// 蒙特卡洛采样参数
		const int SAMPLES = 1000; // 每个探针发射的射线数
		std::mt19937 mt; // 修正随机数生成器名称
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		int bakedCount = 0;
		for (auto entityID : probeView)
		{
			auto [transform, probe] = probeView.get<TransformComponent, LightProbeComponent>(entityID);
			glm::vec3 probePos = transform.Translation;

			// 清空 SH
			for (int i = 0; i < 9; ++i) probe.SHCoefficients[i] = glm::vec3(0.0f);

			// 蒙特卡洛积分
			for (int s = 0; s < SAMPLES; ++s)
			{
				// 生成球面上均匀分布的随机方向
				float u = dist(mt);
				float v = dist(mt);
				float theta = 2.0f * glm::pi<float>() * u;
				float phi = acos(1.0f - 2.0f * v);
				
				glm::vec3 rayDir(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
				
				// 光线追踪 (查找最近交点)
				float minT = std::numeric_limits<float>::max();
				int hitIndex = -1;
				glm::vec3 hitNormal;
				
				for (int b = 0; b < sceneBoxes.size(); ++b) {
					float t; glm::vec3 n;
					if (IntersectAABB(probePos, rayDir, sceneBoxes[b], t, n) && t < minT) {
						minT = t;
						hitIndex = b;
						hitNormal = n;
					}
				}

				glm::vec3 incomingRadiance(0.0f);
				
				// 如果击中了场景物体，计算直接光照作为反弹的辐射度 (1-Bounce)
				if (hitIndex != -1) {
					glm::vec3 hitPos = probePos + rayDir * minT;
					glm::vec3 albedo = sceneBoxes[hitIndex].Albedo;
					
					// 遍历光源计算对该交点的直接光照
					for (const auto& light : lights) {
						glm::vec3 lightDir = light.pos - hitPos;
						float lightDist = glm::length(lightDir);
						lightDir /= lightDist;
						
						// 简单的阴影测试
						bool inShadow = false;
						for (const auto& box : sceneBoxes) {
							float t; glm::vec3 n;
							if (IntersectAABB(hitPos + hitNormal * 0.01f, lightDir, box, t, n) && t < lightDist) {
								inShadow = true;
								break;
							}
						}
						
						if (!inShadow) {
							float ndotl = glm::max(glm::dot(hitNormal, lightDir), 0.0f);
							float attenuation = 1.0f / (1.0f + lightDist * lightDist);
							
							// 由于目前只有一次反弹 (1-Bounce)，我们错失了环境中很多次级反弹的能量。
							// 这里将倍数稍微上调到 3.0，在不过曝的前提下提供充足的辐射度基数。
							float energyMultiplier = 3.0f;
							incomingRadiance += albedo * light.color * ndotl * attenuation * energyMultiplier;
						}
					}
				}
				else {
					// 没击中物体，算作黑色背景
					incomingRadiance = glm::vec3(0.0f);
				}

				// 将采样的 Radiance 投影到 SH 基底并累加
				float sh[9];
				EvalSHBasis(rayDir, sh);
				
				// 蒙特卡洛权重 = 4 * PI / N
				float weight = (4.0f * glm::pi<float>()) / SAMPLES;
				for (int i = 0; i < 9; ++i) {
					probe.SHCoefficients[i] += incomingRadiance * sh[i] * weight;
				}
			}

			probe.IsBaked = true;
			bakedCount++;
		}

		AF_CORE_INFO("LightProbeManager: Baked {0} light probes via RayTracing.", bakedCount);
	}

	std::vector<float> LightProbeManager::GetProbesDataForGPU(Ref<Scene> scene)
	{
		std::vector<float> gpuData;
		auto view = scene->GetAllEntitiesWithView<TransformComponent, LightProbeComponent>();
		
		// 收集到数组中，为了 SSBO 格式对齐：
		// struct GPUProbe { vec4 position_radius; vec4 sh[9]; } // 注意 std430 的 vec3 对齐问题，通常用 vec4

		for (auto entityID : view)
		{
			auto [transform, probe] = view.get<TransformComponent, LightProbeComponent>(entityID);
			
			// 位置和半径
			gpuData.push_back(transform.Translation.x);
			gpuData.push_back(transform.Translation.y);
			gpuData.push_back(transform.Translation.z);
			gpuData.push_back(probe.Radius);

			// 9个 SH 系数 (每个存为 vec4 以满足对齐)
			for (int i = 0; i < 9; ++i)
			{
				gpuData.push_back(probe.SHCoefficients[i].x);
				gpuData.push_back(probe.SHCoefficients[i].y);
				gpuData.push_back(probe.SHCoefficients[i].z);
				gpuData.push_back(0.0f); // padding
			}
		}

		return gpuData;
	}

}
