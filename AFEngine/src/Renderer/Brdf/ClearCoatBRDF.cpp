#include "Renderer/Brdf/ClearCoatBRDF.h"

namespace AF {

std::string ClearCoatBRDF::GenerateGLSL() const
{
    return R"(
// ClearCoat BRDF — 双层: DefaultLit base + ClearCoat top layer
// (Simplified stub — delegates to DefaultLit for now)

vec3 evaluateDirectLight(vec3 N, vec3 V, SurfaceParams surface)
{
    vec3 L = normalize(vec3(0.5, 1.0, 0.3));
    float NdotL = max(dot(N, L), 0.0);
    float clearCoatRoughness = 0.1;
    return surface.BaseColor * NdotL * surface.AO;
}
)";
}

glm::vec3 ClearCoatBRDF::Evaluate(const glm::vec3& N, const glm::vec3& V,
                                   const glm::vec3& L,
                                   const SurfaceParameters& surface) const
{
    return surface.BaseColor * glm::max(glm::dot(N, L), 0.0f) / 3.14159265359f;
}

static bool _regClearCoat = []() {
    BRDF::Register("ClearCoat", std::make_shared<ClearCoatBRDF>());
    return true;
}();

} // namespace AF
