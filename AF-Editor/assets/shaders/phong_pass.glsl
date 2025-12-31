#type vertex
#version 450 core

// 延迟渲染 Phong 光照阶段 - 顶点阶段
// 这是一个全屏四边形着色器

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

// 延迟渲染 Phong 光照阶段 - 片段阶段
// 从 G-Buffer 读取数据并应用 Phong 光照模型和阴影计算

in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

// --- G-Buffer 采样器 ---
uniform sampler2D GBufferAlbedo;  // 基础色
uniform sampler2D GBufferNormal;  // 法线
uniform sampler2D GBufferMP;      // 材质参数 (R: 金属度, G: 粗糙度)
uniform sampler2D GBufferDepth;   // 深度图

// --- 光源结构定义 ---
struct DirLight {
    vec3 direction;          // 方向
    vec3 ambient;            // 环境光
    vec3 diffuse;            // 漫反射
    vec3 specular;           // 镜面反射
    mat4 LightSpaceMatrix;   // 灯光空间变换矩阵 (用于阴影)
};

struct PointLight {
    vec3 position;           // 位置
    vec3 color;              // 颜色
    float intensity;         // 强度
    mat4 LightSpaceMatrix[6]; // 6 个方向的灯光空间矩阵 (用于点光源立方体阴影)
};

// 光源 SSBO
layout(std430, binding = 0) buffer DirLights {
    DirLight dirLights[];
};

layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

// 相机数据 UBO
layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_ViewInverse;
    mat4 u_Projection;
    mat4 u_ProjectionInverse;
};

uniform sampler2D u_EnvMap;               // 环境贴图 (暂未使用)
uniform sampler2DArray DirShadowMap;      // 方向光阴影图数组
uniform samplerCubeArray PointShadowMap;  // 点光源阴影立方体贴图数组

// 根据深度图和逆投影矩阵重建世界空间坐标
vec3 ReconstructWorldPos(vec2 texCoord, float depth) {
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = u_ProjectionInverse * clipPos;
    viewPos /= viewPos.w;
    return (u_ViewInverse * viewPos).xyz;
}

// 方向光阴影计算
float DirShadowCalculation(int lightIndex, vec3 worldPos, vec3 normal, vec3 lightDir) {
    // 忽略背光面
    if (dot(normal, -lightDir) < 0.0)
        return 0.0;

    // 转换到灯光空间坐标
    vec4 lightSpacePos = dirLights[lightIndex].LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5; // 映射到 [0, 1]

    // 超出灯光范围不产生阴影
    if (projCoords.z > 1.0)
        return 0.0;

    // 阴影偏移 (防止阴影失真/痤疮)
    float bias = max(0.05 * (1.0 - dot(normal, -lightDir)), 0.005);
    
    // 采样阴影图
    float closestDepth = texture(DirShadowMap, vec3(projCoords.xy, lightIndex)).r;
    float currentDepth = projCoords.z - bias;
    
    return currentDepth > closestDepth ? 1.0 : 0.0;
}

// 点光源阴影计算
float PointShadowCalculation(int lightIndex, vec3 worldPos, vec3 normal, vec3 lightPos) {
    vec3 fragToLight = worldPos - lightPos;
    float currentDepth = length(fragToLight);

    // 忽略背光面
    if (dot(normal, normalize(-fragToLight)) < 0.0)
        return 0.0;

    float bias = 0.1;
    
    // 采样点光源阴影立方体图
    float closestDepth = texture(PointShadowMap, vec4(normalize(fragToLight), lightIndex)).r;
    
    // 注意: 这里假设阴影图存储的是线性深度
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

// 基础 Phong 光照计算函数
vec3 PhongLighting(vec3 worldPos, vec3 albedo, vec3 normal, float metallic, float roughness, 
                  vec3 lightDir, vec3 lightColor, float shadow) {
    // 环境光 (Ambient)
    vec3 ambient = 0.1 * albedo * lightColor;

    // 漫反射 (Diffuse)
    float diff = max(dot(normal, -lightDir), 0.0);
    vec3 diffuse = diff * albedo * lightColor;

    // 镜面反射 (Specular) - 简化的 Phong
    vec3 viewDir = normalize(u_ViewPos - worldPos);
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0 * (1.0 - roughness));
    vec3 specular = spec * mix(vec3(0.5), albedo, metallic) * lightColor;

    // 应用阴影
    return (ambient + (diffuse + specular) * (1.0 - shadow));
}

void main() {
    // 1. 从 G-Buffer 提取数据
    vec3 albedo = texture(GBufferAlbedo, v_TexCoord).rgb;
    vec3 normal = texture(GBufferNormal, v_TexCoord).rgb;
    vec2 mp = texture(GBufferMP, v_TexCoord).rg; // R: metallic, G: roughness
    float depth = texture(GBufferDepth, v_TexCoord).r;

    float metallic = mp.r;
    float roughness = mp.g;
    
    // 2. 重建世界空间位置
    vec3 worldPos = ReconstructWorldPos(v_TexCoord, depth);
    normal = normalize(normal);

    vec3 totalLight = vec3(0.0);

    // 3. 遍历方向光
    for (int i = 0; i < dirLights.length(); i++) {
        DirLight light = dirLights[i];
        float shadow = DirShadowCalculation(i, worldPos, normal, light.direction);
        totalLight += PhongLighting(worldPos, albedo, normal, metallic, roughness,
                                   light.direction, light.diffuse, shadow);
    }

    // 4. 遍历点光源
    for (int i = 0; i < pointLights.length(); i++) {
        PointLight light = pointLights[i];
        vec3 lightDir = normalize(worldPos - light.position); // 方向: 从灯光到片段
        float shadow = PointShadowCalculation(i, worldPos, normal, light.position);
        
        // 计算简单的距离衰减
        float distance = length(light.position - worldPos);
        float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);
        
        totalLight += PhongLighting(worldPos, albedo, normal, metallic, roughness,
                                   lightDir, light.color * light.intensity, shadow) * attenuation;
    }

    // 5. 输出最终光照颜色
    o_Color = vec4(totalLight, 1.0);
}