#type vertex
#version 450 core

// Phong 渲染着色器 - 顶点阶段
// 处理基础的光照位置和法线变换

layout(location = 0) in vec3 a_Position;   // 顶点位置
layout(location = 1) in vec2 a_Uv;         // 纹理坐标
layout(location = 2) in vec3 a_Normal;     // 顶点法线
layout(location = 3) in vec3 a_Tangent;    // 顶点切线 (此着色器暂未使用)

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;           // 摄像机位置
	mat4 u_ViewProjection;    // 视图投影矩阵
};

uniform mat4 u_Transform;     // 模型变换矩阵

out vec3 v_Normal;            // 传给片段着色器的法线
out vec3 v_FragPos;           // 传给片段着色器的世界空间片段位置
out vec2 v_Uv;                // 传给片段着色器的 UV

void main()
{
    // 计算世界空间坐标
    vec4 worldPosition = u_Transform * vec4(a_Position, 1.0);
    v_FragPos = worldPosition.xyz;
    
    // 计算法线变换矩阵 (使用模型矩阵的逆转置以处理非统一缩放)
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    
    v_Uv = a_Uv;

    // 计算裁剪空间位置
    gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 450 core

// Phong 渲染着色器 - 片段阶段
// 实现基础的 Blinn-Phong 光照模型

layout(location = 0) out vec4 o_Color;    // 颜色输出
layout(location = 1) out int o_EntityID;  // 实体 ID 输出

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_ViewProjection;
};

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_Uv;

uniform sampler2D u_DiffuseMap;   // 漫反射贴图
uniform sampler2D u_SpecularMap;  // 镜面反射贴图
uniform int u_EntityID;           // 实体 ID

// 材质属性
struct Material {
    float shininess;      // 反光度 (指数)
    vec3 specularColor;   // 镜面反射颜色
};
uniform Material u_Material;

// 方向光属性
struct DirLight {
    vec3 direction;       // 光照方向
    vec3 ambient;         // 环境光强度
    vec3 diffuse;         // 漫反射强度
    vec3 specular;        // 镜面反射强度
};

// 方向光 SSBO
layout(std430, binding = 0) buffer DirLights {
    DirLight u_DirLight;
};

void main()
{
    // 1. 获取贴图颜色
    vec3 diffuseColor = texture(u_DiffuseMap, v_Uv).rgb;
    vec3 specularIntensity = texture(u_SpecularMap, v_Uv).rgb;
    
    vec3 normal = normalize(v_Normal);
    
    // 2. 环境光分量 (Ambient)
    vec3 ambient = u_DirLight.ambient * diffuseColor;
    
    // 3. 漫反射分量 (Diffuse)
    vec3 lightDir = normalize(-u_DirLight.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = u_DirLight.diffuse * diff * diffuseColor;
    
    // 4. 镜面反射分量 (Specular) - 使用 Blinn-Phong 模型
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);  // 半程向量
    float spec = pow(max(dot(normal, halfDir), 0.0), u_Material.shininess);

    // 结合贴图强度和材质颜色
    vec3 specular = u_DirLight.specular * spec * specularIntensity * u_Material.specularColor;
    
    // 5. 最终结果合并
    vec3 result = ambient + diffuse + specular;
    
    o_Color = vec4(result, 1.0);
    o_EntityID = u_EntityID;
}