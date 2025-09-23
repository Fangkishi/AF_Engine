#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_Uv;
layout(location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;

out vec3 v_WorldPosition;
out vec2 v_Uv;
out vec3 v_Normal;
out mat3 v_Tbn;

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_ViewProjection;
};
uniform mat4 u_Transform;
uniform mat3 u_NormalMatrix;

void main(){
    vec4 transformPosition = vec4(a_Position, 1.0);
    transformPosition = u_Transform * transformPosition;
    v_WorldPosition = transformPosition.xyz;
    gl_Position = u_ViewProjection * transformPosition;

    v_Uv = a_Uv;
    v_Normal = u_NormalMatrix * a_Normal;
    vec3 tangent = normalize(mat3(u_Transform) * a_Tangent);
    vec3 bitangent = normalize(cross(v_Normal, tangent));
    v_Tbn = mat3(tangent, bitangent, v_Normal);
}

#type fragment
#version 460 core
in vec3 v_WorldPosition;
in vec2 v_Uv;
in vec3 v_Normal;
in mat3 v_Tbn;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_ViewProjection;
};

struct PointLight {
    vec3 position;
    float padding1;  
    vec3 color;
    float intensity;
};

layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

uniform sampler2D u_AlbedoTexture;    // 漫反射贴图
uniform sampler2D u_NormalTexture;    // 法线贴图
uniform sampler2D u_ARMTexture;       // ARM贴图 (R: AO, G: Roughness, B: Metallic)

uniform int u_EntityID;

#define PI 3.141592653589793

// NDF: a = roughness^2
float NDF_GGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);

    float num = a2;
    float denom = PI * (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = max(denom, 0.0001);

    return num / denom;
}

// Geometry函数
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = r * r / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    denom = max(denom, 0.0001);

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel函数
vec3 fresnelSchlick(vec3 F0, float HdotV) {
    return F0 + (1.0 - F0) * pow((1.0 - HdotV), 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, float HdotV, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow((1.0 - HdotV), 5.0);
}

void main() {
    // 1. 从ARM贴图获取材质属性
    vec3 arm = texture(u_ARMTexture, v_Uv).rgb;
    float ambientOcclusion = arm.r;  // R通道环境光遮蔽
    float roughness = arm.g;         // G通道粗糙度
    float metallic = arm.b;          // B通道金属度
    
    // 2. 获取漫反射贴图颜色
    vec3 albedo = texture(u_AlbedoTexture, v_Uv).rgb;
    
    // 3. 法线贴图处理
    vec3 normalMap = texture(u_NormalTexture, v_Uv).rgb;
    normalMap = normalMap * 2.0 - 1.0;  // 从[0,1]转换到[-1,1]
    vec3 N = normalize(v_Tbn * normalMap);

    // 4. 视图方向
    vec3 V = normalize(u_ViewPos - v_WorldPosition);
    float NdotV = max(dot(N, V), 0.0);

    // 5. 计算基础反射率
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // 6. 从SSBO获取光源数量
    int pointLightCount = pointLights.length();
    
    // 7. 对每个光源计算直接光照
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 1; i++) {
        // 7.1 光源方向和半角向量
        vec3 L = normalize(pointLights[i].position - v_WorldPosition);
        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);

        // 7.2 计算光源衰减和辐射率
        float dis = length(pointLights[i].position - v_WorldPosition);
        float attenuation = 1.0 / (dis * dis);
        vec3 irradiance = pointLights[i].color * pointLights[i].intensity * NdotL * attenuation;

        // 7.3 计算BRDF项
        float D = NDF_GGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(F0, max(dot(H, V), 0.0));

        // 7.4 计算反射和漫反射比例
        vec3 ks = F;
        vec3 kd = vec3(1.0 - ks) * (1.0 - metallic);

        // 7.5 计算Cook-Torrance BRDF
        vec3 numerator = D * G * F;
        float denominator = max(4.0 * NdotL * NdotV, 0.0000001);
        vec3 specular = numerator / denominator;

        // 7.6 累积光照贡献
        Lo += (kd * albedo / PI + specular) * irradiance;
    }

    // 8. 计算环境光照，考虑环境光遮蔽
    vec3 ambientKS = fresnelSchlickRoughness(F0, max(NdotV, 0.0), roughness);
    vec3 ambientKD = (1.0 - ambientKS) * (1.0 - metallic);
    vec3 ambient = ambientKD * albedo * 0.1 * ambientOcclusion; // 应用环境光遮蔽

    // 9. 最终颜色输出
    vec3 globalAmbient = vec3(0.03);

    vec3 finalColor = Lo + ambient + globalAmbient;
    o_Color = vec4(finalColor, 1.0);

    o_EntityID = u_EntityID;
}