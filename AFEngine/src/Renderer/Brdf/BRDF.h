#pragma once

// BRDF —— 双向反射分布函数类族
//
// 提供 C++ 端 Evaluate 和 GLSL 代码生成两种能力。
// 通过 Register/Get 静态注册表实现可扩展的 BRDF 模型选择。

#include "Core/Types.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace AF {

struct SurfaceParameters
{
    glm::vec3 BaseColor = { 0.5f, 0.5f, 0.5f };
    float Metallic       = 0.0f;
    float Roughness      = 0.5f;
    float AO             = 1.0f;
};

class BRDF
{
public:
    virtual ~BRDF() = default;
    virtual std::string GetName() const = 0;
    virtual std::string GenerateGLSL() const = 0;
    virtual glm::vec3 Evaluate(const glm::vec3& N, const glm::vec3& V,
                               const glm::vec3& L,
                               const SurfaceParameters& surface) const = 0;

    static void Register(const std::string& name, Ref<BRDF> brdf);
    static Ref<BRDF> Get(const std::string& name);
    static std::vector<std::string> GetAllNames();

private:
    static std::unordered_map<std::string, Ref<BRDF>>& GetRegistry();
};

} // namespace AF
