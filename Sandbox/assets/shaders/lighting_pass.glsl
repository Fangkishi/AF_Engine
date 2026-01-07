#type vertex
#version 450 core

// --- 延迟渲染：PBR 光照阶段顶点着色器 ---
// 负责传递纹理坐标，全屏四边形绘制

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

// --- 延迟渲染：PBR 光照阶段片元着色器 ---
// 核心职责：
// 1. 从 G-Buffer 读取几何与材质数据
// 2. 还原世界空间坐标
// 3. 计算 PBR 光照 (Cook-Torrance BRDF)
// 4. 计算阴影 (方向光 & 点光源)
// 5. 参考 phong_pass.glsl 的数据结构与阴影逻辑

in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

// --- G-Buffer ---
uniform sampler2D GBufferAlbedo;   // 基础色 (Albedo)
uniform sampler2D GBufferNormal;   // 法线 (Normal)
uniform sampler2D GBufferMP;       // 材质属性 (R: Metallic, G: Roughness, B: AO)
uniform sampler2D GBufferDepth;    // 深度图

// --- 阴影贴图 ---
uniform sampler2DArray DirShadowMap;     // 方向光阴影数组 (Layer = LightIndex)
uniform samplerCubeArray PointShadowMap; // 点光源阴影数组 (Layer = LightIndex)

// --- 光源结构 (需与 C++ 侧及 phong_pass.glsl 保持一致) ---
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 LightSpaceMatrix; // 单个矩阵 (无级联 CSM)
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    mat4 LightSpaceMatrix[6]; // 6 个面的变换矩阵
};

// --- SSBO ---
layout(std430, binding = 0) buffer DirLights {
    DirLight dirLights[];
};

layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

// --- Camera UBO ---
layout(std140, binding = 0) uniform Camera {
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_ViewInverse;
    mat4 u_Projection;
    mat4 u_ProjectionInverse;
};

// --- 环境贴图 (IBL) ---
uniform sampler2D u_EnvMap;

// --- 常量 ---
const float PI = 3.14159265359;
const float EPSILON = 0.0001;

// --- PBR 函数 (Cook-Torrance BRDF) ---

// 正态分布函数 (D) - Trowbridge-Reitz GGX
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
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

// 重建世界坐标
vec3 ReconstructWorldPos(vec2 texCoord, float depth) {
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = u_ProjectionInverse * clipPos;
    viewPos /= viewPos.w;
    return (u_ViewInverse * viewPos).xyz;
}

// 世界方向转环境图 UV
vec2 WorldDirToEnvUV(vec3 worldDir) {
    vec3 dir = normalize(worldDir);
    float u = atan(dir.z, dir.x) / (2.0 * PI) + 0.5;
    float v = asin(dir.y) / PI + 0.5;
    return vec2(u, v);
}

// --- 阴影计算 ---

// 计算方向光阴影 (带 PCF)
float CalculateDirShadow(int lightIndex, vec3 worldPos, vec3 N, vec3 L) {
    vec4 lightSpacePos = dirLights[lightIndex].LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // 边界检查
    if (projCoords.z > 1.0 || projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
        return 0.0;

    float bias = max(0.005 * (1.0 - dot(N, L)), 0.0005);
    
    // PCF 3x3 采样
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(DirShadowMap, 0).xy;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(DirShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, lightIndex)).r; 
            shadow += (projCoords.z - bias > pcfDepth ? 1.0 : 0.0);        
        }    
    }
    shadow /= 9.0;
    
    return shadow;
}

// 获取立方体面索引
int GetCubeFaceIndex(vec3 dir) {
    vec3 absDir = abs(dir);
    float maxComponent = max(absDir.x, max(absDir.y, absDir.z));
    if (maxComponent == absDir.x) return dir.x > 0 ? 0 : 1;
    if (maxComponent == absDir.y) return dir.y > 0 ? 2 : 3;
    return dir.z > 0 ? 4 : 5;
}

// 计算点光源阴影
float CalculatePointShadow(int lightIndex, vec3 worldPos, vec3 N, vec3 lightPos) {
    vec3 fragToLight = worldPos - lightPos;
    int faceIndex = GetCubeFaceIndex(fragToLight);
    
    // 使用对应面的投影矩阵将世界坐标转到灯光投影空间
    vec4 lightSpacePos = pointLights[lightIndex].LightSpaceMatrix[faceIndex] * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    float bias = max(0.01 * (1.0 - dot(N, normalize(fragToLight))), 0.001);
    
    // 从 CubeArray 采样深度
    float closestDepth = texture(PointShadowMap, vec4(normalize(fragToLight), lightIndex)).r;
    float currentDepth = projCoords.z - bias;

    return currentDepth > closestDepth ? 1.0 : 0.0;
}

void main() {
    // 1. G-Buffer 读取
    vec3 albedo = texture(GBufferAlbedo, v_TexCoord).rgb;
    vec3 normal = texture(GBufferNormal, v_TexCoord).rgb * 2.0 - 1.0; 
    vec3 mp = texture(GBufferMP, v_TexCoord).rgb;
    float depth = texture(GBufferDepth, v_TexCoord).r;

    // 2. 背景处理 (天空盒)
    if (depth >= 1.0) {
        vec2 uv = v_TexCoord * 2.0 - 1.0;
        vec4 clipPos = vec4(uv, 1.0, 1.0);
        vec4 viewPos = u_ProjectionInverse * clipPos;
        viewPos /= viewPos.w;
        vec3 worldDir = normalize((u_ViewInverse * vec4(viewPos.xyz, 0.0)).xyz);
        o_Color = vec4(texture(u_EnvMap, WorldDirToEnvUV(worldDir)).rgb, 1.0);
        return;
    }

    // 材质参数
    float metallic = mp.r;
    float roughness = max(mp.g, 0.05); // 限制最小粗糙度防止高光过曝
    float ao = mp.b;

    // 3. 场景向量准备
    vec3 worldPos = ReconstructWorldPos(v_TexCoord, depth);
    vec3 N = normalize(normal);
    vec3 V = normalize(u_ViewPos - worldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    // 4. 方向光计算
    for(int i = 0; i < dirLights.length(); ++i) {
        vec3 L = normalize(-dirLights[i].direction);
        vec3 H = normalize(V + L);
        
        float shadow = CalculateDirShadow(i, worldPos, N, L);
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        float NdotL = max(dot(N, L), 0.0);        
        // 使用 diffuse 作为光强/颜色
        Lo += (kD * albedo / PI + specular) * dirLights[i].diffuse * NdotL * (1.0 - shadow); 
    }

    // 5. 点光源计算
    for(int i = 0; i < pointLights.length(); ++i) {
        vec3 L_vec = pointLights[i].position - worldPos;
        vec3 L = normalize(L_vec);
        vec3 H = normalize(V + L);
        
        float distance = length(L_vec);
        // 使用与 phong_pass 一致的衰减公式
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        vec3 radiance = pointLights[i].color * pointLights[i].intensity * attenuation;

        float shadow = CalculatePointShadow(i, worldPos, N, pointLights[i].position);
        
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        float NdotL = max(dot(N, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);
    }

    // 6. 环境光 (IBL 简化，这里仅用常数模拟 AO)
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    // 7. 色调映射 (Reinhard) 与 Gamma 校正
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    o_Color = vec4(color, 1.0);
}
