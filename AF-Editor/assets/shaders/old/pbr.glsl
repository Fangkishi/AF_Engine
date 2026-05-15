#type vertex
#version 460 core

// PBR 渲染管线着色器 - 顶点阶段
// 负责处理顶点位置变换、法线空间转换 (TBN 矩阵) 以及传送到片段着色器的数据

layout(location = 0) in vec3 a_Position;   // 顶点位置
layout(location = 1) in vec2 a_Uv;         // 纹理坐标
layout(location = 2) in vec3 a_Normal;     // 顶点法线
layout(location = 3) in vec3 a_Tangent;    // 顶点切线

out vec3 v_WorldPosition;  // 世界空间下的顶点位置
out vec2 v_Uv;             // 传递给片段着色器的 UV
out vec3 v_Normal;         // 世界空间下的法线
out mat3 v_Tbn;            // 切线空间到世界空间的变换矩阵 (Tangent, Bitangent, Normal)

// 相机数据 UBO
layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;           // 摄像机位置
    mat4 u_ViewProjection;    // 视图投影矩阵
};

uniform mat4 u_Transform;     // 模型变换矩阵
uniform mat3 u_NormalMatrix;  // 法线变换矩阵 (通常是模型矩阵左上角 3x3 的逆转置)

void main(){
    // 计算世界空间坐标
    vec4 transformPosition = vec4(a_Position, 1.0);
    transformPosition = u_Transform * transformPosition;
    v_WorldPosition = transformPosition.xyz;
    
    // 计算裁剪空间坐标
    gl_Position = u_ViewProjection * transformPosition;

    v_Uv = a_Uv;
    
    // 将法线转换到世界空间
    v_Normal = u_NormalMatrix * a_Normal;
    
    // 构建 TBN 矩阵用于法线贴图
    vec3 tangent = normalize(mat3(u_Transform) * a_Tangent);
    vec3 bitangent = normalize(cross(v_Normal, tangent));
    v_Tbn = mat3(tangent, bitangent, v_Normal);
}

#type fragment
#version 460 core

// PBR 渲染管线着色器 - 片段阶段
// 实现基于 Cook-Torrance BRDF 的物理光照模型

in vec3 v_WorldPosition;
in vec2 v_Uv;
in vec3 v_Normal;
in mat3 v_Tbn;

layout(location = 0) out vec4 o_Color;    // 最终颜色输出
layout(location = 1) out int o_EntityID;  // 实体 ID 输出 (用于编辑器拾取)

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_ViewProjection;
};

// 点光源结构体
struct PointLight {
    vec3 position;
    float padding1;  
    vec3 color;
    float intensity;
};

// 点光源 SSBO (支持大量动态光源)
layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

uniform sampler2D u_AlbedoTexture;    // 基础色贴图 (RGB)
uniform sampler2D u_NormalTexture;    // 法线贴图 (RGB)
uniform sampler2D u_ARMTexture;       // ARM 贴图 (R: AO, G: Roughness, B: Metallic)

uniform int u_EntityID;

#define PI 3.141592653589793

// --- PBR 核心函数 ---

// 正态分布函数 (NDF) - Trowbridge-Reitz GGX
// 描述微平面法线分布的集中程度
float NDF_GGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);

    float num = a2;
    float denom = PI * (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = max(denom, 0.0001);

    return num / denom;
}

// 几何函数 (Geometry Function) - Schlick-GGX
// 描述微平面相互遮蔽的概率
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = r * r / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    denom = max(denom, 0.0001);

    return num / denom;
}

// 史密斯法 (Smith's method)
// 结合观察方向和光照方向的几何遮蔽
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 菲涅尔方程 (Fresnel Equation) - Schlick 近似
// 描述光线在不同角度下的反射比率
vec3 fresnelSchlick(vec3 F0, float HdotV) {
    return F0 + (1.0 - F0) * pow((1.0 - HdotV), 5.0);
}

// 考虑粗糙度的菲涅尔近似 (用于环境光)
vec3 fresnelSchlickRoughness(vec3 F0, float HdotV, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow((1.0 - HdotV), 5.0);
}

void main() {
    // 1. 从 ARM 贴图读取材质属性
    vec3 arm = texture(u_ARMTexture, v_Uv).rgb;
    float ambientOcclusion = arm.r;  // R 通道: 环境光遮蔽 (AO)
    float roughness = arm.g;         // G 通道: 粗糙度 (Roughness)
    float metallic = arm.b;          // B 通道: 金属度 (Metallic)
    
    // 2. 获取基础色 (Albedo)
    vec3 albedo = texture(u_AlbedoTexture, v_Uv).rgb;
    
    // 3. 计算切线空间法线
    vec3 normalMap = texture(u_NormalTexture, v_Uv).rgb;
    normalMap = normalMap * 2.0 - 1.0;  // 映射到 [-1, 1]
    vec3 N = normalize(v_Tbn * normalMap);

    // 4. 计算观察向量
    vec3 V = normalize(u_ViewPos - v_WorldPosition);
    float NdotV = max(dot(N, V), 0.0);

    // 5. 计算基础反射率 F0 (非金属默认为 0.04, 金属则使用基础色)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // 6. 光照累加 (Lo)
    vec3 Lo = vec3(0.0);
    
    // 遍历点光源 (目前仅处理第一个作为示例)
    for(int i = 0; i < 1; i++) {
        // 7.1 计算光照向量 L 和半程向量 H
        vec3 L = normalize(pointLights[i].position - v_WorldPosition);
        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);

        // 7.2 计算光照衰减和辐射度
        float dis = length(pointLights[i].position - v_WorldPosition);
        float attenuation = 1.0 / (dis * dis);
        vec3 irradiance = pointLights[i].color * pointLights[i].intensity * NdotL * attenuation;

        // 7.3 计算 Cook-Torrance BRDF 分量
        float D = NDF_GGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(F0, max(dot(H, V), 0.0));

        // 7.4 能量守恒: 漫反射(kd) 和 镜面反射(ks) 的比例
        vec3 ks = F;
        vec3 kd = vec3(1.0 - ks) * (1.0 - metallic);

        // 7.5 计算镜面反射部分
        vec3 numerator = D * G * F;
        float denominator = max(4.0 * NdotL * NdotV, 0.0000001);
        vec3 specular = numerator / denominator;

        // 7.6 累加直接光贡献
        Lo += (kd * albedo / PI + specular) * irradiance;
    }

    // 8. 计算环境光 (Ambient) - 结合 AO
    vec3 ambientKS = fresnelSchlickRoughness(F0, max(NdotV, 0.0), roughness);
    vec3 ambientKD = (1.0 - ambientKS) * (1.0 - metallic);
    vec3 ambient = ambientKD * albedo * 0.1 * ambientOcclusion;

    // 9. 全局背景环境光
    vec3 globalAmbient = vec3(0.03);

    // 10. 最终颜色合成
    vec3 finalColor = Lo + ambient + globalAmbient;
    o_Color = vec4(finalColor, 1.0);

    // 输出实体 ID
    o_EntityID = u_EntityID;
}