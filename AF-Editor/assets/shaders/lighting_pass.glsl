// 延迟渲染 - 光照阶段着色器 (Lighting Pass)
// 负责从 G-Buffer 读取几何信息并计算 PBR 光照

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

// G-Buffer 输入
uniform sampler2D GBufferAlbedo;   // 基础色
uniform sampler2D GBufferNormal;   // 法线
uniform sampler2D GBufferMP;       // 材质参数 (Metallic, Roughness, AO)
uniform sampler2D GBufferDepth;    // 深度图

// 阴影贴图
uniform sampler2DArray DirShadowMap;     // 方向光级联阴影贴图 (CSM)
uniform samplerCubeArray PointShadowMap; // 点光源立方体阴影贴图

// 光源结构定义
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 LightSpaceMatrix[4]; // 每个级联的变换矩阵
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    mat4 LightSpaceMatrix[6]; // 每个面的变换矩阵
};

// 阴影相关常量
const float SHADOW_NEAR = 0.001;
const float SHADOW_FAR = 10.0;

// 光源缓冲区 (SSBO)
layout(std430, binding = 0) buffer DirLights {
    DirLight dirLights[];
};

layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

// 相机 Uniform Block
layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_ViewInverse;
    mat4 u_Projection;
    mat4 u_ProjectionInverse;
};

// 环境贴图
uniform sampler2D u_EnvMap;

const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const float AMBIENT_BOOST = 1.5;   // 环境光增强
const float SPECULAR_CLAMP = 1.0;  // 镜面反射钳制
const float METALLIC_SPECULAR_SCALE = 1.0;

// 阴影偏移和采样常量
const float DIR_SHADOW_BIAS = 0.001;
const float POINT_SHADOW_BIAS = 0.005;
const int PCF_SAMPLE_COUNT = 4;
const float PCF_SAMPLE_RADIUS = 0.002;

// --- PBR 核心函数 ---

// 正态分布函数 (D) - Trowbridge-Reitz GGX
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, EPSILON);
}

// 几何函数 (G) - Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, EPSILON);
}

// 几何项 (G) - Smith 方法
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// 菲涅尔项 (F) - Schlick 近似
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 考虑粗糙度的菲涅尔项
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 从深度值和纹理坐标重建世界空间位置
vec3 ReconstructWorldPosition(vec2 texCoord, float depth) {
    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewSpacePosition = u_ProjectionInverse * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = u_ViewInverse * viewSpacePosition;
    return worldSpacePosition.xyz;
}

// 世界方向转环境图 UV
vec2 WorldDirToEnvUV(vec3 worldDir) {
    vec3 normalizedDir = normalize(worldDir);
    float u = atan(normalizedDir.z, normalizedDir.x) / (2.0 * PI) + 0.5;
    float v = asin(normalizedDir.y) / PI + 0.5;
    return vec2(u, v);
}

// 采样环境图
vec3 SampleEnvironmentMap(vec3 worldDir) {
    vec2 uv = WorldDirToEnvUV(worldDir);
    return texture(u_EnvMap, uv).rgb;
}

// --- 阴影计算函数 ---

// 计算方向光阴影
float CalculateDirShadow(int lightIdx, vec3 worldPos, vec3 N, vec3 L) {
    // 级联选择逻辑 (目前默认使用级联 0)
    int cascadeIdx = 0;
    cascadeIdx = clamp(cascadeIdx, 0, 3);

    // 转换到灯光空间
    mat4 lightSpaceMat = dirLights[lightIdx].LightSpaceMatrix[cascadeIdx];
    vec4 lightSpacePos = lightSpaceMat * vec4(worldPos, 1.0);
    vec3 ndcPos = lightSpacePos.xyz / lightSpacePos.w;
    
    // 范围检查
    if (ndcPos.x < -1.0 || ndcPos.x > 1.0 || 
        ndcPos.y < -1.0 || ndcPos.y > 1.0 || 
        ndcPos.z < -1.0 || ndcPos.z > 1.0) {
        return 1.0;
    }
    
    vec2 shadowTexCoord = ndcPos.xy * 0.5 + 0.5;
    float currentDepth = ndcPos.z;

    // 应用偏移防止阴影痤疮
    float bias = max(DIR_SHADOW_BIAS * (1.0 - dot(N, L)), DIR_SHADOW_BIAS * 0.1);
    currentDepth -= bias;

    // PCF 过滤
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(DirShadowMap, 0).xy;
    int sampleCount = 0;
    
    const vec2[] spiralSamples = vec2[] (
        vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1), vec2(-1, 1),
        vec2(-1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1), vec2(2, 0)
    );
    
    for (int i = 0; i < 10; i++) {
        vec2 offset = spiralSamples[i] * PCF_SAMPLE_COUNT;
        vec2 sampleCoord = shadowTexCoord + offset * texelSize * PCF_SAMPLE_RADIUS;
        
        float pcfDepth = texture(DirShadowMap, vec3(sampleCoord, lightIdx * 4 + cascadeIdx)).r;
        shadow += (currentDepth > pcfDepth) ? 1.0 : 0.0;
        sampleCount++;
    }
    
    return 1.0 - (shadow / float(sampleCount));
}

// 计算点光源阴影
float CalculatePointShadow(int lightIdx, vec3 worldPos, vec3 N) {
    vec3 lightPos = pointLights[lightIdx].position;
    vec3 lightToWorld = worldPos - lightPos;
    float lightToWorldDist = length(lightToWorld);
    
    if (lightToWorldDist > SHADOW_FAR) {
        return 1.0;
    }

    float shadow = 0.0;
    int sampleCount = 0;
    vec3 sampleOffsets[12] = vec3[] (
        vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
        vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3(1, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0), vec3(0, -1, 0)
    );
    
    float offsetRadius = PCF_SAMPLE_RADIUS * (lightToWorldDist / SHADOW_FAR);
    offsetRadius = min(offsetRadius, 0.05);
    
    vec3 dir = normalize(lightToWorld);
    
    for (int i = 0; i < 12; i++) {
        vec3 sampleDir = normalize(dir + sampleOffsets[i] * offsetRadius);
        float shadowMapDepth = texture(PointShadowMap, vec4(sampleDir, lightIdx)).r;
        
        // 线性化深度值
        float linearShadowDepth = SHADOW_NEAR * SHADOW_FAR / 
            (SHADOW_FAR - shadowMapDepth * (SHADOW_FAR - SHADOW_NEAR));
        
        float bias = POINT_SHADOW_BIAS * (1.0 - dot(N, -sampleDir)) * (lightToWorldDist / SHADOW_FAR);
        bias = max(bias, 0.001);
        
        float currentDepth = lightToWorldDist - bias;
        float depthTolerance = 0.005 * lightToWorldDist;
        
        shadow += (currentDepth > linearShadowDepth + depthTolerance) ? 1.0 : 0.0;
        sampleCount++;
    }

    return 1.0 - (shadow / float(sampleCount));
}

void main()
{
    // 1. 从 G-Buffer 采样数据
    vec4 albedoData = texture(GBufferAlbedo, v_TexCoord);
    vec3 albedo = albedoData.rgb;
    float alpha = albedoData.a;

    vec3 normal = texture(GBufferNormal, v_TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0); // 解压法线

    vec3 materialData = texture(GBufferMP, v_TexCoord).rgb;
    float metallic = materialData.r;
    float roughness = max(materialData.g, 0.05);
    float ao = materialData.b;

    float depth = texture(GBufferDepth, v_TexCoord).r;

    // 2. 处理背景/天空盒
    if (depth >= 0.9999) {
        vec2 uv = v_TexCoord * 2.0 - 1.0;
        vec4 clipPos = vec4(uv, 1.0, 1.0);
        vec4 viewPos = u_ProjectionInverse * vec4(clipPos.xy, 1.0, 1.0);
        viewPos /= viewPos.w;
        vec3 worldDir = (u_ViewInverse * vec4(viewPos.xyz, 0.0)).xyz;
        worldDir = normalize(worldDir);
    
        vec3 skyColor = texture(u_EnvMap, WorldDirToEnvUV(worldDir)).rgb;
        o_Color = vec4(skyColor, 1.0);
        return;
    }

    // 3. 重建光照计算所需的向量
    vec3 worldPos = ReconstructWorldPosition(v_TexCoord, depth);
    vec3 N = normalize(normal);
    vec3 V = normalize(u_ViewPos - worldPos);
    
    // 基础反射率 F0
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    F0 = mix(F0, albedo * 0.7, metallic);
    
    vec3 Lo = vec3(0.0);

    // 4. 计算直接光照
    if (dirLights.length() > 0 || pointLights.length() > 0) {
        // 方向光
        for (int i = 0; i < dirLights.length(); ++i) {
            vec3 L = normalize(-dirLights[i].direction);
            vec3 H = normalize(V + L);
            float NdotL = max(dot(N, L), 0.0);
        
            if (NdotL > 0.0) {
                float NDF = DistributionGGX(N, H, roughness);
                float G = GeometrySmith(N, V, L, roughness);
                vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
            
                vec3 kS = F;
                vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
            
                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + EPSILON;
                vec3 specular = numerator / denominator;
            
                if (metallic < 0.5)
                    specular = clamp(specular, 0.0, SPECULAR_CLAMP);
            
                vec3 diffuse = kD * albedo / PI;
                float dirShadowFactor = CalculateDirShadow(i, worldPos, N, L);
                
                Lo += (diffuse + specular) * dirLights[i].diffuse * dirShadowFactor * NdotL;
            }
        }

        // 点光源
        for (int i = 0; i < pointLights.length(); ++i) {
            vec3 L_vec = pointLights[i].position - worldPos;
            float distance = length(L_vec);
            vec3 L = normalize(L_vec);
            vec3 H = normalize(V + L);
            float NdotL = max(dot(N, L), 0.0);
        
            if (NdotL > 0.0) {
                float attenuation = 1.0 / (distance * distance);
                float falloff = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);
                vec3 radiance = pointLights[i].color * pointLights[i].intensity * attenuation * falloff;
            
                float NDF = DistributionGGX(N, H, roughness);
                float G = GeometrySmith(N, V, L, roughness);
                vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
            
                vec3 kS = F;
                vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
            
                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + EPSILON;
                vec3 specular = numerator / denominator;
            
                if (metallic < 0.5)
                    specular = clamp(specular, 0.0, SPECULAR_CLAMP);
            
                vec3 diffuse = kD * albedo / PI;
                float pointShadowFactor = CalculatePointShadow(i, worldPos, N);

                Lo += (diffuse + specular) * radiance * NdotL * pointShadowFactor;
            }
        }
    }

    // 5. 环境光处理 (IBL 简化版)
    vec3 ambient = vec3(0.3) * albedo * ao * AMBIENT_BOOST;

    // 6. 汇总颜色并进行色调映射 (Tonemapping)
    vec3 color = ambient + Lo;
    
    // Reinhard 色调映射
    color = color / (color + vec3(1.0));
    // Gamma 校正
    color = pow(color, vec3(1.0 / 2.2));
    
    o_Color = vec4(color, alpha);
}
