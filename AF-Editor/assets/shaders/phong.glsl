#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_Uv;

layout(std140, binding = 1) uniform Camera
{
    vec3 u_ViewPos;
	mat4 u_ViewProjection;
};
uniform mat4 u_Transform;

out vec3 v_Normal;
out vec3 v_FragPos;
out vec2 v_Uv;

void main()
{
    vec4 worldPosition = u_Transform * vec4(a_Position, 1.0);
    v_FragPos = worldPosition.xyz;
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    v_Uv = a_Uv;

    gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(std140, binding = 1) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_ViewProjection;
};

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_Uv;

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SpecularMap;
uniform int u_EntityID;

struct Material {
    float shininess;
    vec3 specularColor;
};
uniform Material u_Material;

struct DirLight {
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight u_DirLight;

void main()
{
    vec3 diffuseColor = texture(u_DiffuseMap, v_Uv).rgb;
    vec3 specularMapValue = texture(u_SpecularMap, v_Uv).rgb;
    
    vec3 normal = normalize(v_Normal);
    
    // 环境光
    vec3 ambient = u_DirLight.ambient * diffuseColor;
    
    // 漫反射
    vec3 lightDir = normalize(-u_DirLight.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = u_DirLight.diffuse * diff * diffuseColor;
    
    // 镜面反射 - 使用 Blinn-Phong 模型（性能更好）
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), u_Material.shininess);

    // 使用镜面贴图的RGB通道分别控制不同方面
    vec3 specularIntensity = texture(u_SpecularMap, v_Uv).rgb;
    vec3 specular = u_DirLight.specular * spec * specularIntensity * u_Material.specularColor;
    
    vec3 result = ambient + diffuse + specular;
    
    o_Color = vec4(result, 1.0);
    o_EntityID = u_EntityID;
}