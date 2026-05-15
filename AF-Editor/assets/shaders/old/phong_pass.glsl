#type vertex
#version 450 core

// --- 延迟渲染：全屏光照阶段顶点着色器 ---
// 负责渲染覆盖全屏的四边形，将纹理坐标传递给片元着色器。

layout(location = 0) in vec3 a_Position;   // 顶点位置
layout(location = 1) in vec2 a_TexCoord;   // 纹理坐标

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

// --- 延迟渲染：全屏光照阶段片元着色器 ---
// 核心职责：
// 1. 从 G-Buffer 读取几何与材质数据。
// 2. 重建片段的世界空间坐标。
// 3. 计算多光源的阴影贡献。
// 4. 应用 Phong 光照模型并输出最终颜色。

in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

// --- G-Buffer 采样器绑定 ---
uniform sampler2D GBufferAlbedo;  // 颜色 / 基础色
uniform sampler2D GBufferNormal;  // 世界空间法线
uniform sampler2D GBufferMP;      // 材质属性 (R: 金属度, G: 粗糙度)
uniform sampler2D GBufferDepth;   // 硬件深度图

// --- 数据结构定义 ---
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 LightSpaceMatrix;   // 用于阴影采样的灯光空间变换矩阵
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    mat4 LightSpaceMatrix[6]; // 用于点光源阴影的 6 个方向变换矩阵
};

// --- SSBO & UBO 绑定 ---
layout(std430, binding = 0) buffer DirLights {
    DirLight dirLights[];
};

layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

layout(std140, binding = 0) uniform Camera {
    vec3 u_ViewPos;           // 摄像机世界坐标
    mat4 u_View;
    mat4 u_ViewInverse;
    mat4 u_Projection;
    mat4 u_ProjectionInverse; // 用于重建世界空间坐标
};

// --- 阴影资源绑定 ---
uniform sampler2D u_EnvMap;               // 环境映射 (IBL 占位)
uniform sampler2DArray DirShadowMap;      // 方向光阴影贴图数组 (每个图层对应一个光源)
uniform samplerCubeArray PointShadowMap;  // 点光源阴影贴图数组 (每个立方体对应一个光源)

const float PI = 3.14159265359;

vec3 SampleSphericalMap(vec3 v)
{
    float u = atan(v.z, v.x) / (2.0 * PI) + 0.5;
    float v_uv = asin(v.y) / PI + 0.5;
    return texture(u_EnvMap, vec2(u, v_uv)).rgb;
}

/**
 * @brief 根据深度图重建世界空间坐标
 */
vec3 ReconstructWorldPos(vec2 texCoord, float depth) {
    // OpenGL 深度范围 [0, 1] -> NDC 范围 [-1, 1]
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = u_ProjectionInverse * clipPos;
    viewPos /= viewPos.w; // 透视除法
    return (u_ViewInverse * viewPos).xyz;
}

/**
 * @brief 计算方向光阴影
 */
float DirShadowCalculation(int lightIndex, vec3 worldPos, vec3 normal, vec3 lightDir) {
    // 变换到灯光空间
    vec4 lightSpacePos = dirLights[lightIndex].LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5; // 映射到 [0, 1] 范围

    // 处理超出边界的情况
    if (projCoords.z > 1.0 || projCoords.z < 0.0 || 
        projCoords.x > 1.0 || projCoords.x < 0.0 || 
        projCoords.y > 1.0 || projCoords.y < 0.0)
        return 0.0;

    // 计算动态 Bias 解决阴影失真 (Shadow Acne)
    float bias = max(0.005 * (1.0 - dot(normal, -lightDir)), 0.0005);
    
    // 从 Texture Array 采样深度
    float closestDepth = texture(DirShadowMap, vec3(projCoords.xy, lightIndex)).r;
    float currentDepth = projCoords.z - bias;
    
    return currentDepth > closestDepth ? 1.0 : 0.0;
}

/**
 * @brief 获取点光源阴影对应的立方体面索引
 */
int GetCubeFaceIndex(vec3 dir) {
    vec3 absDir = abs(dir);
    float maxComponent = max(absDir.x, max(absDir.y, absDir.z));
    if (maxComponent == absDir.x) return dir.x > 0 ? 0 : 1; // +X, -X
    if (maxComponent == absDir.y) return dir.y > 0 ? 2 : 3; // +Y, -Y
    return dir.z > 0 ? 4 : 5; // +Z, -Z
}

/**
 * @brief 计算点光源阴影
 */
float PointShadowCalculation(int lightIndex, vec3 worldPos, vec3 normal, vec3 lightPos) {
    vec3 fragToLight = worldPos - lightPos;
    int faceIndex = GetCubeFaceIndex(fragToLight);
    
    // 使用对应面的变换矩阵计算当前片段深度
    vec4 lightSpacePos = pointLights[lightIndex].LightSpaceMatrix[faceIndex] * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // 从 samplerCubeArray 采样深度
    float closestDepth = texture(PointShadowMap, vec4(normalize(fragToLight), lightIndex)).r;
    
    float bias = max(0.01 * (1.0 - dot(normal, normalize(fragToLight))), 0.001);
    float currentDepth = projCoords.z - bias;
    
    return currentDepth > closestDepth ? 1.0 : 0.0;
}

/**
 * @brief Phong 光照模型计算
 */
vec3 PhongLighting(vec3 worldPos, vec3 albedo, vec3 normal, float metallic, float roughness, 
                  vec3 lightDir, vec3 lightColor, float shadow) {
    // 1. 环境光
    vec3 ambient = 0.05 * albedo * lightColor;

    // 2. 漫反射
    float diff = max(dot(normal, -lightDir), 0.0);
    vec3 diffuse = diff * albedo * lightColor;

    // 3. 镜面反射 (基于 roughness 调整高光指数)
    vec3 viewDir = normalize(u_ViewPos - worldPos);
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mix(1.0, 128.0, 1.0 - roughness));
    vec3 specular = spec * mix(vec3(0.04), albedo, metallic) * lightColor;

    // 4. 合并并应用阴影
    return (ambient + (diffuse + specular) * (1.0 - shadow));
}

void main() {
    // 1. 读取 G-Buffer 数据
    vec3 albedo = texture(GBufferAlbedo, v_TexCoord).rgb;
    vec3 normal = texture(GBufferNormal, v_TexCoord).rgb * 2.0 - 1.0; // 解包法线
    vec2 mp = texture(GBufferMP, v_TexCoord).rg; 
    float depth = texture(GBufferDepth, v_TexCoord).r;

    // 2. 边界检查 (背景处理)
    if (depth >= 1.0) {
        // 重建远平面世界坐标以计算视线方向
        vec3 worldPos = ReconstructWorldPos(v_TexCoord, 1.0);
        vec3 viewDir = normalize(worldPos - u_ViewPos);
        
        // 采样环境贴图作为背景
        vec3 envColor = SampleSphericalMap(normalize(viewDir));
        o_Color = vec4(envColor, 1.0);
        return;
    }

    float metallic = mp.r;
    float roughness = mp.g;
    
    // 3. 场景信息准备
    vec3 worldPos = ReconstructWorldPos(v_TexCoord, depth);
    normal = normalize(normal);

    vec3 totalLight = vec3(0.0);

    // 4. 处理所有方向光
    for (int i = 0; i < dirLights.length(); i++) {
        DirLight light = dirLights[i];
        float shadow = DirShadowCalculation(i, worldPos, normal, light.direction);
        totalLight += PhongLighting(worldPos, albedo, normal, metallic, roughness,
                                   light.direction, light.diffuse, shadow);
    }

    // 5. 处理所有点光源 (包含阴影与距离衰减)
    for (int i = 0; i < pointLights.length(); i++) {
        PointLight light = pointLights[i];
        vec3 lightToFrag = worldPos - light.position;
        vec3 lightDir = normalize(lightToFrag);
        float shadow = PointShadowCalculation(i, worldPos, normal, light.position);
        
        float distance = length(lightToFrag);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        totalLight += PhongLighting(worldPos, albedo, normal, metallic, roughness,
                                   lightDir, light.color * light.intensity, shadow) * attenuation;
    }

    // 6. 最终颜色输出
    o_Color = vec4(totalLight, 1.0);
}
