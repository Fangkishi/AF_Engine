#include "Renderer/Brdf/DefaultLitBRDF.h"
#include <sstream>

namespace AF {

std::string DefaultLitBRDF::GenerateGLSL() const
{
    return R"(
// DefaultLit BRDF — Cook-Torrance + Lambert diffuse
// GGX NDF, Smith Geometry, Schlick Fresnel

struct SurfaceParams {
    vec3 BaseColor;
    float Roughness;
    float Metallic;
    float AO;
};

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 EvaluateBRDF(vec3 N, vec3 V, vec3 L, SurfaceParams surface)
{
    vec3 H = normalize(V + L);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    vec3 F0 = mix(vec3(0.04), surface.BaseColor, surface.Metallic);

    // Cook-Torrance specular
    float D = DistributionGGX(N, H, surface.Roughness);
    float G = GeometrySmith(N, V, L, surface.Roughness);
    vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);

    // Lambert diffuse with energy conservation
    vec3 kD = (vec3(1.0) - F) * (1.0 - surface.Metallic);
    vec3 diffuse = kD * surface.BaseColor / PI;

    return (diffuse + specular) * surface.AO * NdotL;
}

vec3 evaluateDirectLight(vec3 N, vec3 V, SurfaceParams surface)
{
    vec3 L = normalize(vec3(0.5, 1.0, 0.3));
    return EvaluateBRDF(N, V, L, surface);
}
)";
}

glm::vec3 DefaultLitBRDF::Evaluate(const glm::vec3& N, const glm::vec3& V,
                                    const glm::vec3& L,
                                    const SurfaceParameters& surface) const
{
    float NdotL = glm::max(glm::dot(N, L), 0.0f);
    return surface.BaseColor * NdotL * surface.AO / 3.14159265359f;
}

static bool _regDefaultLit = []() {
    BRDF::Register("DefaultLit", std::make_shared<DefaultLitBRDF>());
    return true;
}();

} // namespace AF
