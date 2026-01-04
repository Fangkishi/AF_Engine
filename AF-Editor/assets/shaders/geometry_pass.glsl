// 延迟渲染 - 几何阶段着色器 (Geometry Pass)
// 负责将场景的几何信息渲染到 G-Buffer 中

#type vertex
#version 450 core

// 顶点属性输入
layout(location = 0) in vec3 a_Position; // 位置
layout(location = 1) in vec3 a_Normal;   // 法线
layout(location = 2) in vec3 a_Tangent;  // 切线
layout(location = 3) in vec3 a_Bitangent;// 副切线
layout(location = 4) in vec2 a_TexCoord; // 纹理坐标

// 相机 Uniform Block
layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_ViewInverse;
    mat4 u_Projection;
    mat4 u_ProjectionInverse;
};

// 传递给片段着色器的数据
struct VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec3 Tangent;
    vec3 Bitangent;
    vec2 TexCoord;
};

layout(location = 0) out VertexOutput Output;
layout(location = 5) out flat int v_EntityID;

// 物体变换矩阵和法线矩阵
uniform mat4 u_Transform;
uniform mat3 u_NormalMatrix;
uniform int u_EntityID;

void main()
{
    // 计算世界空间坐标
    vec4 worldPosition = u_Transform * vec4(a_Position, 1.0);
    Output.WorldPosition = worldPosition.xyz;
    
    // 计算世界空间下的法线、切线、副切线
    Output.Normal = normalize(u_NormalMatrix * a_Normal);
    Output.Tangent = normalize(u_NormalMatrix * a_Tangent);
    Output.Bitangent = normalize(u_NormalMatrix * a_Bitangent);
    
    Output.TexCoord = a_TexCoord;
    v_EntityID = u_EntityID;

    // 计算裁剪空间坐标
    gl_Position = u_Projection * u_View * worldPosition;
}

#type fragment
#version 450 core

// G-Buffer 输出
layout(location = 0) out vec4 g_Albedo;         // 漫反射/基础色 (RGB) + 透明度 (A)
layout(location = 1) out int g_EntityID;        // 实体 ID
layout(location = 2) out vec4 g_Normal;         // 世界空间法线 (压缩到 [0, 1])
layout(location = 3) out vec4 g_MaterialParams; // 材质参数: 金属度 (R), 粗糙度 (G), 环境遮蔽 (B)

struct VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec3 Tangent;
    vec3 Bitangent;
    vec2 TexCoord;
};

layout(location = 0) in VertexOutput Input;
layout(location = 5) in flat int v_EntityID;

// 材质属性结构
struct Material
{
    vec4 AlbedoColor;
    float Metallic;
    float Roughness;
    float AmbientOcclusion;
    int UseAlbedoMap;
    int UseNormalMap;
    int UseMetallicMap;
    int UseRoughnessMap;
    int UseAOMap;
};

uniform Material u_Material;

// 各种贴图
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;
uniform sampler2D u_ARMMap; // 环境遮蔽、粗糙度、金属度混合贴图

// 计算切线空间到世界空间的变换矩阵 (TBN)
mat3 CalculateTBNMatrix()
{
    vec3 N = normalize(Input.Normal);
    vec3 T = normalize(Input.Tangent);
    vec3 B = normalize(Input.Bitangent);
    
    // 施密特正交化
    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);

    return mat3(T, B, N);
}

// 采样法线贴图
vec3 SampleNormalMap()
{
    // 增加贴图有效性检查，防止在未绑定贴图时采样到错误的纹理单元（如帧缓冲残留）
    if (u_Material.UseNormalMap == 0 || textureSize(u_NormalMap, 0).x <= 1)
        return normalize(Input.Normal);
    
    // 从法线贴图采样并转换到 [-1, 1] 范围
    vec3 tangentNormal = texture(u_NormalMap, Input.TexCoord).xyz * 2.0 - 1.0;

    // 基础校验
    float normalLength = length(tangentNormal);
    if (normalLength < 0.5 || normalLength > 2.0) {
        return normalize(Input.Normal);
    }
    
    tangentNormal = normalize(tangentNormal);
    
    if (any(isnan(tangentNormal)) || length(tangentNormal) < 0.1) {
        return normalize(Input.Normal);
    }

    // 转换到世界空间
    mat3 TBN = CalculateTBNMatrix();
    vec3 worldNormal = TBN * tangentNormal;
    return normalize(worldNormal);
}

// 采样基础色
vec4 SampleAlbedo()
{
    vec4 albedo = u_Material.AlbedoColor;
    // 如果启用贴图，且贴图单元有效（尺寸 > 1），则进行采样
    if (u_Material.UseAlbedoMap == 1 && textureSize(u_AlbedoMap, 0).x > 1)
    {
        albedo *= texture(u_AlbedoMap, Input.TexCoord);
    }
    return albedo;
}

// 采样金属度
float SampleMetallic()
{
    // 1. 优先使用独立金属度贴图 (如果启用且单元有效)
    if (u_Material.UseMetallicMap == 1 && textureSize(u_MetallicMap, 0).x > 1)
    {
        return clamp(texture(u_MetallicMap, Input.TexCoord).r, 0.0, 1.0);
    }
    // 2. 其次尝试使用 ARM 贴图 (如果启用且单元有效)
    else if (u_Material.UseMetallicMap == 1 && textureSize(u_ARMMap, 0).x > 1)
    {
        return clamp(texture(u_ARMMap, Input.TexCoord).b, 0.0, 1.0);
    }
    
    // 3. 最后使用材质参数数值
    return clamp(u_Material.Metallic, 0.0, 1.0);
}

// 采样粗糙度
float SampleRoughness()
{
    // 1. 优先使用独立粗糙度贴图 (如果启用且单元有效)
    if (u_Material.UseRoughnessMap == 1 && textureSize(u_RoughnessMap, 0).x > 1)
    {
        return clamp(texture(u_RoughnessMap, Input.TexCoord).r, 0.01, 1.0);
    }
    // 2. 其次尝试使用 ARM 贴图 (如果启用且单元有效)
    else if (u_Material.UseRoughnessMap == 1 && textureSize(u_ARMMap, 0).x > 1)
    {
        return clamp(texture(u_ARMMap, Input.TexCoord).g, 0.01, 1.0);
    }

    // 3. 最后使用材质参数数值
    return clamp(u_Material.Roughness, 0.01, 1.0);
}

// 采样环境遮蔽 (AO)
float SampleAmbientOcclusion()
{
    // 1. 优先使用独立 AO 贴图 (如果启用且单元有效)
    if (u_Material.UseAOMap == 1 && textureSize(u_AOMap, 0).x > 1)
    {
        return clamp(texture(u_AOMap, Input.TexCoord).r, 0.0, 1.0);
    }
    // 2. 其次尝试使用 ARM 贴图 (如果启用且单元有效)
    else if (u_Material.UseAOMap == 1 && textureSize(u_ARMMap, 0).x > 1)
    {
        return clamp(texture(u_ARMMap, Input.TexCoord).r, 0.0, 1.0);
    }

    // 3. 最后使用材质参数数值
    return clamp(u_Material.AmbientOcclusion, 0.0, 1.0);
}

void main()
{
    // 1. 处理基础色和透明剔除
    vec4 albedo = SampleAlbedo();
    if (albedo.a < 0.1)
        discard;
    
    g_Albedo = vec4(albedo.rgb, albedo.a);
    g_EntityID = v_EntityID;
    
    // 2. 处理法线并压缩到 [0, 1] 范围进行存储
    vec3 worldNormal = SampleNormalMap();
    g_Normal = vec4(normalize(worldNormal) * 0.5 + 0.5, 1.0);
    
    // 3. 处理材质参数
    float metallic = SampleMetallic();
    float roughness = SampleRoughness();
    float ambientOcclusion = SampleAmbientOcclusion();

    g_MaterialParams = vec4(metallic, roughness, ambientOcclusion, 1.0);
}
