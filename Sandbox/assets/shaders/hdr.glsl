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

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform sampler2D u_SceneTexture;
uniform float u_Exposure = 1.0;

void main()
{
    vec4 texColor = texture(u_SceneTexture, v_TexCoord);
    
    // 应用曝光处理
    vec3 exposedColor = texColor.rgb * u_Exposure;
    
    // Reinhard色调映射
    vec3 mapped = exposedColor / (exposedColor + vec3(1.0));
    
    // Gamma校正
    mapped = pow(mapped, vec3(1.0 / 2.2));
    
    o_Color = vec4(texColor.rgb, 1.0);
}