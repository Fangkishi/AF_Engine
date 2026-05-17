#pragma once

// ClearCoatBRDF —— 清漆层 BRDF
//
// 双层材质：底层 DefaultLit + 顶层 ClearCoat 层。
// 当前为简化桩实现（委托给 DefaultLit）。静态初始化时自动注册。

#include "Renderer/Brdf/BRDF.h"

namespace AF {

class ClearCoatBRDF : public BRDF
{
public:
    std::string GetName() const override { return "ClearCoat"; }
    std::string GenerateGLSL() const override;
    glm::vec3 Evaluate(const glm::vec3& N, const glm::vec3& V,
                       const glm::vec3& L,
                       const SurfaceParameters& surface) const override;
};

} // namespace AF
