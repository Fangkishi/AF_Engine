#pragma once

// DefaultLitBRDF —— Cook-Torrance 微表面 PBR 模型
//
// 包含 GGX NDF、Smith 几何项和 Schlick Fresnel 近似。
// 在静态初始化时自动向 BRDF 注册表注册。

#include "Renderer/Brdf/BRDF.h"

namespace AF {

class DefaultLitBRDF : public BRDF
{
public:
    std::string GetName() const override { return "DefaultLit"; }
    std::string GenerateGLSL() const override;
    glm::vec3 Evaluate(const glm::vec3& N, const glm::vec3& V,
                       const glm::vec3& L,
                       const SurfaceParameters& surface) const override;
};

} // namespace AF
