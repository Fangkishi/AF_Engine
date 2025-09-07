#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_Uv;

layout(std140, binding = 0) uniform Camera
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

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_ViewProjection;
};

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_Uv;

uniform sampler2D u_DiffuseMap;
uniform int u_EntityID;

struct DirLight {
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight u_DirLight;

void main()
{
    vec3 color = texture(u_DiffuseMap, v_Uv).rgb;
    vec3 normal = normalize(v_Normal);
    
    // 环境光
    vec3 ambient = u_DirLight.ambient * color;
    
    // 漫反射
    vec3 lightDir = normalize(-u_DirLight.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = u_DirLight.diffuse * diff * color;
    
    // 镜面反射
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = u_DirLight.specular * spec;
    
    vec3 result = ambient + diffuse + specular;
    
    o_Color = vec4(result, 1.0);
    o_EntityID = u_EntityID;
}